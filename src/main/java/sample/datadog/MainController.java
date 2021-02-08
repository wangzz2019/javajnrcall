package sample.datadog;

import java.time.LocalDateTime;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;
import java.util.logging.Logger;

import datadog.opentracing.DDTracer.DDTracerBuilder;
import datadog.trace.api.Config;
import datadog.trace.api.DDTags;
//import datadog.trace.api.GlobalTracer;
import datadog.trace.common.writer.DDAgentWriter;
import io.opentracing.propagation.Format;
import io.opentracing.propagation.TextMap;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RestController;

import datadog.trace.api.Trace;
import io.micrometer.core.annotation.Timed;
import jnr.ffi.LibraryLoader;
import lombok.extern.slf4j.Slf4j;

import datadog.opentracing.DDTracer;
import io.opentracing.*;
import io.opentracing.util.GlobalTracer;

class MyTraceInfo {
	Map<String, String> m;
	MyTraceInfo(){
		m=new HashMap<String,String>();
	}
	void addData(String k, String v) {
		m.put(k,v);
	}
}

@RestController
@Slf4j
@Timed
public class MainController implements InitializingBean {
	private INative iNative;

	@RequestMapping(value = "/sprinttest", method = RequestMethod.GET)
	public String springtest(){
		Tracer tracer=GlobalTracer.get();
		Span span = tracer.buildSpan("spring_test")
//				.withTag(DDTags.SERVICE_NAME, "spring_test_service")
				.start();
		try (Scope scope = tracer.activateSpan(span)) {
			// Tags can also be set after creation
			span.setTag("my.tag", "value");

			// The code you’re tracing

		} catch (Exception e) {
			// Set error on span
		} finally {
			// Close span in a finally block
			span.finish();
		}
		System.out.println("it is just a test");
		return "";
	}
	/**
	 *
	 * @return
	 */
	@RequestMapping(value = "/test", method = RequestMethod.GET)
	@Timed(value = "time.logging.callnative")
//	@Trace(operationName = "trace.logging.callnative", resourceName = "MainController.loggingWithNative")
	public String loggingWithNative() {
//		DDTracer tracer=DDTracer.builder().writer(DDAgentWriter.builder().build()).build();
		Tracer tracer= GlobalTracer.get();
		Span span = tracer.buildSpan("javajnrcall").start();
//		ScopeManager sm=tracer.scopeManager();
//		Tracer.SpanBuilder tb=tracer.buildSpan("javajnrcall");
//		Span span=tb.start();
//		try (Scope scope=sm.activate(span)){
		try (Scope scope=tracer.activateSpan(span)){
//			String tid=tracer.getTraceId();
//			String spanid=tracer.getSpanId();
			span.setTag(DDTags.SERVICE_NAME, "java2cpp-0.0.1-snapshot");
			// span.setTag(DDTags.SERVICE_NAME, "cppservice");
			MyTraceInfo mytraceinfo=new MyTraceInfo();
			tracer.inject(span.context(),
					Format.Builtin.TEXT_MAP,
					new MyTraceInfoInjectAdapter(mytraceinfo));

			for (String key : mytraceinfo.m.keySet()){
				log.info(key + " = " + mytraceinfo.m.get(key));
			}
			// convert hashmap to Array
			String[] keys = mytraceinfo.m.keySet().toArray(new String[0]);
			String[] values=new String[keys.length];
			int idx=0;
			for (String key:keys){
				values[idx++]=mytraceinfo.m.get(key);
			}

			log.info("CALLING C++ from Java >>>>>>>>>>");
			int cppResult=iNative.runcppkv(2,4,keys,values);

			assert cppResult == 6;
			log.info("<<<<<<<<<< C++ FINISHED. Result={}", cppResult);
		} finally {
			span.finish();
//			tracer.close();
		}
		return "finished. " + LocalDateTime.now().toString();
	}

	@Override
	public void afterPropertiesSet() throws Exception {
		// a file named "libplus.{dll, dylib, so}" should be set on the path "-Djava.library.path".
		this.iNative = LibraryLoader.create(INative.class).load("plus");
	}
}
