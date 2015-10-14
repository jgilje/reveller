package jgilje.net.reveller;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

/**
 * Created by jgilje on 10/11/15.
 */
public class SidListFragment extends Fragment {
    private ClickListener clickListener;
    private List<String> directories;
    private List<String> files;
    private String path;
    private SimpleStringRecyclerViewAdapter adapter;

    public interface ClickListener {
        void file(String file);
        void folder(String folder);
    }

    void setClickListener(ClickListener listener) {
        this.clickListener = listener;
    }

    void update(String path, List<String> directories, List<String> files) {
        this.path = path;
        this.files = files;
        this.directories = directories;

        adapter.notifyDataSetChanged();
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        RecyclerView rv = (RecyclerView) inflater.inflate(R.layout.fragment_sid_list, container, false);
        setupRecyclerView(rv);
        return rv;
    }

    private void setupRecyclerView(RecyclerView recyclerView) {
        adapter = new SimpleStringRecyclerViewAdapter(getActivity());
        recyclerView.setLayoutManager(new LinearLayoutManager(recyclerView.getContext()));
        recyclerView.setAdapter(adapter);
    }

    public class SimpleStringRecyclerViewAdapter extends RecyclerView.Adapter<SimpleStringRecyclerViewAdapter.ViewHolder> {
        private final TypedValue mTypedValue = new TypedValue();
        private int mBackground;

        public class ViewHolder extends RecyclerView.ViewHolder {
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

        private boolean positionIsFile(int position) {
            if (position >= directories.size()) {
                return  true;
            }

            return false;
        }

        public String getValueAt(int position) {
            if (positionIsFile(position)) {
                return files.get(position - directories.size());
            } else {
                return directories.get(position);
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
            holder.mBoundString = getValueAt(position);
            holder.mTextView.setText(holder.mBoundString);
            if (positionIsFile(position)) {
                holder.mImageView.setImageResource(R.drawable.ic_audiotrack_black_24dp);
            } else {
                holder.mImageView.setImageResource(R.drawable.ic_folder_black_24dp);
            }

            holder.mView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (positionIsFile(position)) {
                        clickListener.file(path + '/' + files.get(position - directories.size()));
                    } else {
                        clickListener.folder(path + '/' + directories.get(position));
                    }
                }
            });
        }

        @Override
        public int getItemCount() {
            return directories.size() + files.size();
        }
    }
}
