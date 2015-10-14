package jgilje.net.reveller;

import android.app.Dialog;
import android.app.DialogFragment;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

/**
 * Created by jgilje on 10/10/15.
 */
public class ManualEntryFragment extends DialogFragment {
    public interface Listener {
        void onManualEntry(String host, int port);
    }

    private Listener listener;

    private ManualEntryFragment() {}

    public static ManualEntryFragment newInstance(Listener listener) {
        ManualEntryFragment fragment = new ManualEntryFragment();
        fragment.listener = listener;
        return fragment;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        // Get the layout inflater
        LayoutInflater inflater = getActivity().getLayoutInflater();

        // Inflate and set the layout for the dialog
        // Pass null as the parent view because its going in the dialog layout
        final View view = inflater.inflate(R.layout.fragment_manual_entry, null);
        builder.setView(view)
                .setPositiveButton(R.string.add, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        TextView host = (TextView) view.findViewById(R.id.edit_text_host);
                        TextView port = (TextView) view.findViewById(R.id.edit_text_port);
                        listener.onManualEntry(host.getText().toString(), Integer.parseInt(port.getText().toString()));
                    }
                })
                .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        ManualEntryFragment.this.getDialog().cancel();
                    }
                });

        return builder.create();
    }
}
