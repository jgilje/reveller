package jgilje.net.reveller;

import android.content.Context;
import android.content.Intent;
import android.support.v7.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by jgilje on 2/20/16.
 */
public class RecyclerViewAdapter extends RecyclerView.Adapter<RecyclerViewAdapter.ViewHolder> {
    private final TypedValue mTypedValue = new TypedValue();
    private List<RevellerEntry> entries = new ArrayList<>();
    private int mBackground;

    public static class ViewHolder extends RecyclerView.ViewHolder {
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

    public RevellerEntry getEntry(int position) {
        return entries.get(position);
    }

    public void addEntry(RevellerEntry entry) {
        int pos = entries.size();
        entries.add(entry);
        notifyItemInserted(pos);
    }

    public void removeEntry(int position) {
        entries.remove(position);
        notifyItemRemoved(position);
    }

    public void clearEntries() {
        int items = entries.size();
        entries.clear();
        notifyItemRangeRemoved(0, items);
    }

    public RecyclerViewAdapter(Context context) {
        context.getTheme().resolveAttribute(R.attr.selectableItemBackground, mTypedValue, true);
        mBackground = mTypedValue.resourceId;
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item, parent, false);
        view.setBackgroundResource(mBackground);

        view.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                RecyclerView rv = (RecyclerView) v.getParent();
                int i = rv.getChildAdapterPosition(v);
                RevellerEntry entry = entries.get(i);

                Context context = v.getContext();
                Intent intent = new Intent(context, MainActivity.class);
                intent.putExtra(MainActivity.SERVERHOST_EXTRA, entry.host);
                intent.putExtra(MainActivity.SERVERPORT_EXTRA, entry.port);
                context.startActivity(intent);
            }
        });

        view.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                RecyclerView rv = (RecyclerView) v.getParent();
                int i = rv.getChildAdapterPosition(v);
                RevellerEntry entry = entries.get(i);

                // only manual entries get context-menu, to remove the entry
                if (entry.manual) {
                    v.showContextMenu();
                }

                return true;
            }
        });

        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(final ViewHolder holder, final int position) {
        RevellerEntry entry = entries.get(position);
        if (entry.manual) {
            holder.mTextView.setText(String.format("%s:%d", entry.host, entry.port));
        } else {
            holder.mTextView.setText(entry.host);
        }
        holder.mImageView.setImageResource(R.drawable.ic_developer_board_black_24dp);
    }

    @Override
    public int getItemCount() {
        return entries.size();
    }
}
