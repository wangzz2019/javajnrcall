package sample.datadog;

import io.opentracing.*;

/**
 * interface for native language
 */
public interface INative {
	// function name, arguments and return value
	// should be same with those of native module.
	int plus(int m, int n);
	int runcpp(int m, int n);
	//int runcpp(int m, int n, Span span);
	int runcpp(int m, int n, String tid, String sid);
	int runcppkv(int m, int n, String[] keys, String[] values);
}