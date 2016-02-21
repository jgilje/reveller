package jgilje.net.reveller;

import java.io.Serializable;
import java.util.Objects;

/**
 * Created by jgilje on 2/20/16.
 */
public class RevellerEntry implements Serializable {
    private static final long serialVersionUID = 0L;

    public transient String displayName = null;
    public String host = null;
    public int port = 0;
    public boolean manual;

    public RevellerEntry(String displayName, String host, int port) {
        this.displayName = displayName;
        this.host = host;
        this.port = port;
        this.manual = false;
    }

    public RevellerEntry(String host, int port) {
        this.host = host;
        this.port = port;
        this.manual = true;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        RevellerEntry that = (RevellerEntry) o;
        return port == that.port &&
                Objects.equals(host, that.host);
    }

    @Override
    public int hashCode() {
        return Objects.hash(host, port);
    }
}
