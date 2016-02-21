package jgilje.net.reveller;

import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.ContextMenu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.HashSet;
import java.util.Set;
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

    private RecyclerViewAdapter adapter;

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
                        RevellerEntry entry = new RevellerEntry(host, port);
                        Set<RevellerEntry> manualEntries = getManualEntries();
                        manualEntries.add(entry);
                        saveManualEntries(manualEntries);
                        adapter.addEntry(entry);
                    }
                });
                newFragment.show(ft, "manual_entry_dialog");
            }
        });

        ContextMenuRecyclerView rv = (ContextMenuRecyclerView) findViewById(R.id.recyclerview);
        setupRecyclerView(rv);

        nsdManager = (NsdManager) getApplicationContext().getSystemService(Context.NSD_SERVICE);
    }

    private Set<RevellerEntry> getManualEntries() {
        try {
            FileInputStream fis = openFileInput(getString(R.string.manuel_entries_file));
            ObjectInputStream ois = new ObjectInputStream(fis);
            Set<RevellerEntry> entries = (Set<RevellerEntry>) ois.readObject();
            return entries;
        } catch (Throwable e) {
            return new HashSet<>();
        }
    }

    private void saveManualEntries(Set<RevellerEntry> entries) {
        try {
            FileOutputStream fos = openFileOutput(getString(R.string.manuel_entries_file), Context.MODE_PRIVATE);
            ObjectOutputStream oos = new ObjectOutputStream(fos);
            oos.writeObject(entries);
        } catch (Throwable e) {
            System.out.println(e);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

        nsdManager.stopServiceDiscovery(discoveryListener);
    }

    @Override
    protected void onResume() {
        super.onResume();

        for (RevellerEntry entry : getManualEntries()) {
            adapter.addEntry(entry);
        }

        initializeDiscoveryListener();
    }

    private void setupRecyclerView(RecyclerView recyclerView) {
        recyclerView.setLayoutManager(new LinearLayoutManager(recyclerView.getContext()));
        adapter = new RecyclerViewAdapter(getApplicationContext());
        recyclerView.setAdapter(adapter);
        registerForContextMenu(recyclerView);
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.list_context_menu, menu);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        ContextMenuRecyclerView.ContextMenuInfo info = (ContextMenuRecyclerView.ContextMenuInfo) item.getMenuInfo();

        switch (item.getItemId()) {
            case R.id.remove:
                RevellerEntry entry = adapter.getEntry(info.position);
                Set<RevellerEntry> manualEntries = getManualEntries();
                manualEntries.remove(entry);
                saveManualEntries(manualEntries);
                adapter.removeEntry(info.position);
                return true;
            default:
                return super.onContextItemSelected(item);
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

                final String serviceName = nsdServiceInfo.getServiceName();
                final String canonicalHostName = nsdServiceInfo.getHost().getCanonicalHostName();
                final int port = nsdServiceInfo.getPort();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        adapter.addEntry(new RevellerEntry(serviceName, canonicalHostName, port));
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
                        adapter.clearEntries();
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
