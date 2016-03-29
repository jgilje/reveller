package jgilje.net.reveller;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.AttributeSet;
import android.view.ContextMenu;
import android.view.View;

/**
 * Created by jgilje on 2/20/16.
 */
public class ContextMenuRecyclerView extends RecyclerView {
    private ContextMenuInfo mContextMenuInfo = new ContextMenuInfo();

    public ContextMenuRecyclerView(Context context) {
        super(context);
    }

    public ContextMenuRecyclerView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ContextMenuRecyclerView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public boolean showContextMenuForChild(View originalView) {
        int position = getChildAdapterPosition(originalView);
        mContextMenuInfo.position = position;
        return super.showContextMenuForChild(originalView);
    }

    @Override
    protected ContextMenu.ContextMenuInfo getContextMenuInfo() {
        return mContextMenuInfo;
    }

    public static class ContextMenuInfo implements ContextMenu.ContextMenuInfo {
        public int position;
    }
}
