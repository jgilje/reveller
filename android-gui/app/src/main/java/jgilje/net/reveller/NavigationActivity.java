package jgilje.net.reveller;

import android.app.Activity;
import android.os.Bundle;

/**
 * Created by jgilje on 5/24/15.
 */
public class NavigationActivity extends Activity {
    public static final String CONNECTION_HOST = "CONNECTION_HOST";
    public static final String CONNECTION_PORT = "CONNECTION_PORT";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_navigation);
    }
}
