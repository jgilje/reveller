package jgilje.net.reveller;

import android.os.Bundle;
import android.support.design.widget.NavigationView;
import android.support.design.widget.TabLayout;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;

import org.java_websocket.client.WebSocketClient;
import org.java_websocket.drafts.Draft_17;
import org.java_websocket.handshake.ServerHandshake;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by jgilje on 5/24/15.
 */
public class MainActivity extends AppCompatActivity {
    private static final String TAG = "ConnectActivity";
    public static final String SERVERHOST_EXTRA = "SERVERHOST_EXTRA";
    public static final String SERVERPORT_EXTRA = "SERVERPORT_EXTRA";

    private DrawerLayout mDrawerLayout;
    private String serverHost;
    private int serverPort;
    private WebSocketClient webSocketClient;
    private View nowPlayingFrame;
    private Adapter adapter;
    private ViewPager viewPager;
    private TabLayout tabLayout;

    private SidListFragment.ClickListener listener = new SidListFragment.ClickListener() {
        @Override
        public void file(String file) {
            try {
                JSONObject json = new JSONObject();
                json.put("action", "load");
                json.put("argument", file);
                webSocketClient.send(json.toString());

                json.put("action", "song");
                json.put("argument", "0");
                webSocketClient.send(json.toString());
            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }

        @Override
        public void folder(String folder) {
            try {
                JSONObject json = new JSONObject();
                json.put("action", "ls");
                json.put("argument", folder);
                webSocketClient.send(json.toString());
            } catch (JSONException e) {
                throw new RuntimeException(e);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        final ActionBar ab = getSupportActionBar();
        ab.setHomeAsUpIndicator(R.drawable.ic_menu);
        ab.setDisplayHomeAsUpEnabled(true);

        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);

        NavigationView navigationView = (NavigationView) findViewById(R.id.nav_view);
        if (navigationView != null) {
            setupDrawerContent(navigationView);
        }

        viewPager = (ViewPager) findViewById(R.id.viewpager);
        tabLayout = (TabLayout) findViewById(R.id.tabs);
        if (viewPager != null) {
            setupViewPager(viewPager);
            tabLayout.setupWithViewPager(viewPager);
        }

        nowPlayingFrame = findViewById(R.id.now_playing_frame);

        serverHost = getIntent().getStringExtra(SERVERHOST_EXTRA);
        serverPort = getIntent().getIntExtra(SERVERPORT_EXTRA, -1);
    }

    @Override
    protected void onResume() {
        super.onResume();
        connectWebSocket();
    }

    private void setupViewPager(ViewPager viewPager) {
        adapter = new Adapter(getSupportFragmentManager());
        viewPager.setAdapter(adapter);
    }

    private void setupDrawerContent(NavigationView navigationView) {
        navigationView.setNavigationItemSelectedListener(
                new NavigationView.OnNavigationItemSelectedListener() {
                    @Override
                    public boolean onNavigationItemSelected(MenuItem menuItem) {
                        menuItem.setChecked(true);
                        mDrawerLayout.closeDrawers();
                        return true;
                    }
                });
    }

    class Adapter extends FragmentPagerAdapter {
        private final List<Fragment> mFragments = new ArrayList<>();
        private final List<String> mFragmentTitles = new ArrayList<>();

        public Adapter(FragmentManager fm) {
            super(fm);
        }

        public void setPath(String path, List<String> directories, List<String> files) {
            String[] splits = path.split("/");

            SidListFragment fragment;
            if (mFragments.size() < splits.length) {
                fragment = new SidListFragment();
                fragment.setClickListener(listener);
                mFragments.add(fragment);

                notifyDataSetChanged();
            } else {
                fragment = (SidListFragment) mFragments.get(splits.length - 1);
            }

            if ((tabLayout.getSelectedTabPosition() + 1) < tabLayout.getTabCount()) {
                int selected = tabLayout.getSelectedTabPosition() + 1;
                int extras = tabLayout.getTabCount() - selected;
                for (int i = (extras - 1); i >= 0; i--) {
                    int pos = selected + i;
                    tabLayout.removeTabAt(pos);
                }
            }

            TabLayout.Tab tab = tabLayout.newTab();
            if (path.isEmpty()) {
                tab.setText("ROOT");
            } else {
                tab.setText(splits[splits.length - 1]);
            }

            tabLayout.addTab(tab, true);

            fragment.update(path, directories, files);
            notifyDataSetChanged();
        }

        @Override
        public Fragment getItem(int position) {
            return mFragments.get(position);
        }

        @Override
        public int getCount() {
            return mFragments.size();
        }

        @Override
        public CharSequence getPageTitle(int position) {
            return mFragmentTitles.get(position);
        }
    }

    private void connectWebSocket() {
        URI httpUri;
        URI wsUri;
        try {
            httpUri = new URI("http://" + serverHost + ":"  + serverPort);
            wsUri = new URI(String.format("ws://%s:%s/ws", httpUri.getHost(), httpUri.getPort()));
        } catch (URISyntaxException e) {
            e.printStackTrace();
            return;
        }

        Map<String,String> headers = new HashMap<>();
        headers.put("Origin", httpUri.toString());
        webSocketClient = new WebSocketClient(wsUri, new Draft_17(), headers, 0) {
            @Override
            public void onOpen(ServerHandshake serverHandshake) {
                Log.i("Websocket", "Opened");
                JSONObject json = new JSONObject();
                try {
                    json.put("action", "state");
                    webSocketClient.send(json.toString());

                    json.put("action", "currentHeader");
                    webSocketClient.send(json.toString());

                    json.put("action", "ls");
                    webSocketClient.send(json.toString());
                } catch (JSONException e) {
                    throw new RuntimeException(e);
                }
            }

            @Override
            public void onMessage(final String message) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        final JSONObject rootObject;
                        try {
                            rootObject = (JSONObject) new JSONTokener(message).nextValue();
                            String type = rootObject.getString("type");

                            switch (type) {
                                case "state":
                                    JSONObject stateObject = (JSONObject) new JSONTokener(rootObject.getString("data")).nextValue();
                                    handleState(stateObject);
                                    break;
                                case "load":
                                    handleLoad(rootObject.getString("data"));
                                    break;
                                case "currentSidHeader":
                                    JSONObject sidHeaderObject = (JSONObject) new JSONTokener(rootObject.getString("data")).nextValue();
                                    handleCurrentSidHeader(sidHeaderObject);
                                    break;
                                case "ls":
                                    JSONObject lsObject = (JSONObject) new JSONTokener(rootObject.getString("data")).nextValue();
                                    handleLs(lsObject);
                                    break;
                                default:
                                    Log.w("UnhandledReplyType", type);
                                    break;
                            }
                        } catch (JSONException e) {
                            throw new RuntimeException(e);
                        }
                    }
                });
            }

            @Override
            public void onClose(int i, String s, boolean b) {
                Log.i("Websocket", "Closed " + s);
            }

            @Override
            public void onError(Exception e) {
                Log.i("Websocket", "Error " + e.getMessage());
            }
        };

        webSocketClient.connect();
    }

    /**
     * Example {"type":"state","data":"{\"file\":\"MUSICIANS/B/Brimble_Allister/Spellbound_Dizzy.sid\",\"state\":\"play\",\"song\":1}"}
     */
    public void handleState(JSONObject stateObject) throws JSONException {
        ImageButton playPauseButton = (ImageButton) nowPlayingFrame.findViewById(R.id.now_playing_button_playpause);
        String state = stateObject.getString("state");
        switch (state) {
            case "play":
                playPauseButton.setImageDrawable(getResources().getDrawable(android.R.drawable.ic_media_pause));
                break;
            case "stop":
                playPauseButton.setImageDrawable(getResources().getDrawable(android.R.drawable.ic_media_play));
                break;
            default:
                Log.w("SidControl", "Unknown state: " + state);
        }
    }

    /**
     * Example {"type":"load","data":"MUSICIANS/D/Dune/Raven_note.sid"}
     */
    public void handleLoad(String loadedFile) {

    }

    /**
     * Example {"type":"currentSidHeader","data":"{\"Type\":\"PSID\",\"Version\":2,\"DataOffset\":124,\"LoadAddress\":4096,\"InitAddress\":4096,\"PlayAddress\":4099,\"Songs\":1,\"StartSong\":1,\"Speed\":[\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\",\"VBI\"],\"Name\":\"The Raven (note)\",\"Author\":\"Rafal Pietraszko (Dune)\",\"Released\":\"1995 Oxygen/Black Picture\",\"Flags\":{\"InternalPlayer\":true,\"ComputePlayer\":false,\"C64Compatible\":true,\"PlaySIDSamples\":false,\"BASIC_ROM\":false,\"Speed\":\"PAL\",\"SIDModel\":\"8580\"},\"StartPage\":0,\"PageLength\":0,\"Hz\":985248}"}
     */
    public void handleCurrentSidHeader(JSONObject sidHeaderObject) throws JSONException {
        TextView title = (TextView) nowPlayingFrame.findViewById(R.id.now_playing_title);
        TextView author = (TextView) nowPlayingFrame.findViewById(R.id.now_playing_author);

        title.setText(sidHeaderObject.getString("Name"));
        author.setText(sidHeaderObject.getString("Author"));
    }

    /**
     * Example {"path":"","directories":["DEMOS","DOCUMENTS","GAMES","MUSICIANS","update"],"sidfiles":[]}
     */
    public void handleLs(JSONObject lsObject) throws JSONException {
        String path = lsObject.getString("path");
        List<String> directories = jsonArrayToStringList(lsObject.getJSONArray("directories"));
        List<String> files = jsonArrayToStringList(lsObject.getJSONArray("sidfiles"));

        /*
        String title = "";
        int lastIdx = path.lastIndexOf('/');
        if (lastIdx >= 0) {
            title = path.substring(lastIdx + 1);
        }
        */
        adapter.setPath(path, directories, files);
    }

    private List<String> jsonArrayToStringList(JSONArray array) throws JSONException {
        List<String> list = new ArrayList<>(array.length());
        for (int i = 0; i < array.length(); i++) {
            list.add(array.getString(i));
        }

        return list;
    }
}
