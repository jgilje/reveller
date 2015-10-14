package jgilje.net.reveller;

import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

/**
 * Created by jgilje on 10/10/15.
 */
public class ConnectActivity extends AppCompatActivity {
    /**
     * Log tag
     */
    private static final String TAG = "ConnectActivity";

    /**
     * The service name on mDNS
     */
    private static final String SERVICE_TYPE = "_reveller._tcp";

    private NsdManager.DiscoveryListener discoveryListener;

    private NsdManager.ResolveListener resolveListener;

    private NsdManager nsdManager;

    private SimpleStringRecyclerViewAdapter serviceAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connect);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                FragmentTransaction ft = getFragmentManager().beginTransaction();
                Fragment prev = getFragmentManager().findFragmentByTag("manual_entry_dialog");
                if (prev != null) {
                    ft.remove(prev);
                }
                ft.addToBackStack(null);

                // Create and show the dialog.
                DialogFragment newFragment = ManualEntryFragment.newInstance(new ManualEntryFragment.Listener() {
                    @Override
                    public void onManualEntry(String host, int port) {

                    }
                });
                newFragment.show(ft, "manual_entry_dialog");
            }
        });

        RecyclerView rv = (RecyclerView) findViewById(R.id.recyclerview);
        setupRecyclerView(rv);

        nsdManager = (NsdManager) getApplicationContext().getSystemService(Context.NSD_SERVICE);
        serviceAdapter.addEntry("reveller-pi.gilje.fluxxx.lan", 8080);
    }

    @Override
    protected void onPause() {
        super.onPause();

        nsdManager.stopServiceDiscovery(discoveryListener);
    }

    @Override
    protected void onResume() {
        super.onResume();

        initializeDiscoveryListener();
    }

    private void setupRecyclerView(RecyclerView recyclerView) {
        recyclerView.setLayoutManager(new LinearLayoutManager(recyclerView.getContext()));
        serviceAdapter = new SimpleStringRecyclerViewAdapter(getApplicationContext());
        recyclerView.setAdapter(serviceAdapter);
    }

    public static class SimpleStringRecyclerViewAdapter extends RecyclerView.Adapter<SimpleStringRecyclerViewAdapter.ViewHolder> {
        private final TypedValue mTypedValue = new TypedValue();
        private int mBackground;
        private List<String> mHostnames = new ArrayList<>();
        private List<Integer> mPorts = new ArrayList<>();

        public void addEntry(String canonicalHostName, int port) {
            int pos = mHostnames.size();
            mHostnames.add(canonicalHostName);
            mPorts.add(port);

            notifyItemInserted(pos);
        }

        public void clearEntries() {
            int items = mHostnames.size();
            mHostnames.clear();
            mPorts.clear();

            notifyItemRangeRemoved(0, items);
        }

        public static class ViewHolder extends RecyclerView.ViewHolder {
            public String mBoundString;

            public final View mView;
            public final ImageView mImageView;
            public final TextView mTextView;

            public ViewHolder(View view) {
                super(view);
                mView = view;
                mImageView = (ImageView) view.findViewById(R.id.list_item_icon);
                mTextView = (TextView) view.findViewById(R.id.list_item_text);
            }

            @Override
            public String toString() {
                return super.toString() + " '" + mTextView.getText();
            }
        }

        public SimpleStringRecyclerViewAdapter(Context context) {
            context.getTheme().resolveAttribute(R.attr.selectableItemBackground, mTypedValue, true);
            mBackground = mTypedValue.resourceId;
        }

        @Override
        public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
            View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item, parent, false);
            view.setBackgroundResource(mBackground);
            return new ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(final ViewHolder holder, final int position) {
            holder.mBoundString = mHostnames.get(position);
            holder.mTextView.setText(mHostnames.get(position));

            holder.mView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Context context = v.getContext();
                    Intent intent = new Intent(context, MainActivity.class);
                    intent.putExtra(MainActivity.SERVERHOST_EXTRA, mHostnames.get(position));
                    intent.putExtra(MainActivity.SERVERPORT_EXTRA, mPorts.get(position));

                    context.startActivity(intent);
                }
            });

            holder.mImageView.setImageResource(R.drawable.ic_developer_board_black_24dp);
        }

        @Override
        public int getItemCount() {
            return mHostnames.size();
        }
    }

    private void initializeDiscoveryListener() {
        // Instantiate a new DiscoveryListener
        final Vector<NsdServiceInfo> queue = new Vector<>();

        resolveListener = new NsdManager.ResolveListener() {
            @Override
            public void onResolveFailed(NsdServiceInfo nsdServiceInfo, int i) {
                System.out.println("Failed resolve");
            }

            @Override
            public void onServiceResolved(NsdServiceInfo nsdServiceInfo) {
                queue.remove(0);
                if (queue.size() > 0) {
                    nsdManager.resolveService(queue.get(0), resolveListener);
                }

                final String canonicalHostName = nsdServiceInfo.getHost().getCanonicalHostName();
                final int port = nsdServiceInfo.getPort();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        serviceAdapter.addEntry(canonicalHostName, port);
                    }
                });
            }
        };

        discoveryListener = new NsdManager.DiscoveryListener() {
            //  Called as soon as service discovery begins.
            @Override
            public void onDiscoveryStarted(String regType) {
                Log.d(TAG, "Service discovery started");
            }

            @Override
            public void onServiceFound(NsdServiceInfo nsdServiceInfo) {
                queue.add(nsdServiceInfo);
                if (queue.size() == 1) {
                    nsdManager.resolveService(nsdServiceInfo, resolveListener);
                }
            }

            @Override
            public void onServiceLost(NsdServiceInfo service) {
                // When the network service is no longer available.
                // Internal bookkeeping code goes here.
                Log.e(TAG, "service lost" + service);
            }

            @Override
            public void onDiscoveryStopped(String serviceType) {
                Log.i(TAG, "Discovery stopped: " + serviceType);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        serviceAdapter.clearEntries();
                    }
                });
            }

            @Override
            public void onStartDiscoveryFailed(String serviceType, int errorCode) {
                Log.e(TAG, "Discovery failed: Error code:" + errorCode);
                nsdManager.stopServiceDiscovery(this);
            }

            @Override
            public void onStopDiscoveryFailed(String serviceType, int errorCode) {
                Log.e(TAG, "Discovery failed: Error code:" + errorCode);
                nsdManager.stopServiceDiscovery(this);
            }
        };

        nsdManager.discoverServices(SERVICE_TYPE, NsdManager.PROTOCOL_DNS_SD, discoveryListener);
    }
}
