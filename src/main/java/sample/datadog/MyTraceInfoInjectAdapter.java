package sample.datadog;

import io.opentracing.propagation.TextMap;

import java.util.Iterator;
import java.util.Map;

public class MyTraceInfoInjectAdapter implements TextMap {
    private final MyTraceInfo myTraceInfo;

    public MyTraceInfoInjectAdapter(final MyTraceInfo myTraceInfo) {
        this.myTraceInfo = myTraceInfo;
    }

    @Override
    public void put(final String key, final String value) {
        this.myTraceInfo.addData(key,value);
    }

    @Override
    public Iterator<Map.Entry<String, String>> iterator() {
        throw new UnsupportedOperationException("This class should be used only with tracer#inject()");
    }
}
