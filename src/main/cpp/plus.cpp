#include <stdio.h>
#include <datadog/opentracing.h>
//#include <datadog/span.h>
//#include <datadog/tracer_options.h>
#include <opentracing/tracer.h>
//#include <opentracing/span.h>
#include <opentracing/propagation.h>
//#include <opentracing/noop.h>
#include <iostream>
#include <string>
using namespace datadog::opentracing;

extern "C" int runcpp(int m, int n, char* t, char* s);
extern "C" int runcppkv(int m, int n, char** keys, char** values);
extern "C" int plus(int m, int n);


struct CustomCarrierReader : opentracing::TextMapReader {
      explicit CustomCarrierReader(
          const std::unordered_map<std::string, std::string>& data_)
          : data{data_} {}
    
      using F = std::function<opentracing::expected<void>(
          opentracing::string_view, opentracing::string_view)>;
    
      opentracing::expected<void> ForeachKey(F f) const override {
        // Iterate through all key-value pairs, the tracer will use the relevant keys
        // to extract a span context.
        for (auto& key_value : data) {
          auto was_successful = f(key_value.first, key_value.second);
          if (!was_successful) {
            // If the callback returns and unexpected value, bail out of the loop.
            return was_successful;
          }
        }
    
        // Indicate successful iteration.
        return {};
      }

      // Optional, define TextMapReader::LookupKey to allow for faster extraction.
      opentracing::expected<opentracing::string_view> LookupKey(
          opentracing::string_view key) const override {
        auto iter = data.find(key);
        if (iter != data.end()) {
          return opentracing::make_unexpected(opentracing::key_not_found_error);
        }
        return opentracing::string_view{iter->second};
      }

      const std::unordered_map<std::string, std::string>& data;
    };

int runcppkv(int m, int n, char** keys, char** values){
  int result;
  printf("Printing key-values pairs in CPP:\n");
  for (int i=0;*(keys+i)!=nullptr;i++){
    std::cout<<*(keys+i)<<": "<<*(values+i)<<std::endl;
  }
  datadog::opentracing::TracerOptions tracer_options{"localhost", 8126, "cppservice"};
  auto tracer = datadog::opentracing::makeTracer(tracer_options);
  {
    std::unordered_map<std::string, std::string> text_map;
    for (int i=0;*(keys+i)!=nullptr;i++){
      text_map[std::string(*(keys+i))]=std::string(*(values+i));
    }

    CustomCarrierReader carrier(text_map);
    auto span_context=tracer->Extract(carrier);
    auto span=tracer->StartSpan("nativeCode", {ChildOf(span_context->get())});

    span->SetTag("cppspan", 123);
    result= plus(m,n);
    span->Finish();
  }
  tracer->Close();
  return result;
}

int runcpp(int m, int n,char* t,char* s){
  int result;
  printf("Printing sid in CPP:\n");
  for (int i = 0; s[i] != '\0'; i++) {
  	printf("%c", s[i]);
  }
  printf("\n");
  printf("Printing tid in CPP:\n");
  for (int i = 0; t[i] != '\0'; i++) {
    	printf("%c", t[i]);
    }
  printf("\n");

  datadog::opentracing::TracerOptions tracer_options{"localhost", 8126, "cppservice"};
  auto tracer = datadog::opentracing::makeTracer(tracer_options);
  //auto tracer = opentracing::Tracer::Global();
  // if (opentracing::Tracer::InitGlobal(tracer)!=nullptr) {printf("Global regist ok\n");}
  // const ot::StartSpanOptions span_options;

  {
    //auto span=tracer->StartSpanWithOptions("cppspan",span_options);
    std::unordered_map<std::string, std::string> text_map;
    text_map["x-datadog-parent-id"]=std::string(s);
    text_map["x-datadog-sampling-priority"]=std::string("1");
    text_map["x-datadog-trace-id"]=std::string(t);

    CustomCarrierReader carrier(text_map);
    auto span_context=tracer->Extract(carrier);
    auto span=tracer->StartSpan("nativeCode", {ChildOf(span_context->get())});

    //auto childspan = tracer->StartSpan("plusspan",{opentracing::ChildOf(&activespan.context())});
    //childspan->SetTag("cppspan", 123);
    span->SetTag("cppspan", 123);
    result= plus(m,n);
    //const ot::FinishSpanOptions finish_options;
    //span_a->FinishWithOptions(finish_options);
    span->Finish();
  }
  tracer->Close();
  return result;
}

int plus(int m, int n){
	int result = m + n;
	
	printf("C++ called!! This message printed by CPP! result=%d\n", result);
	fflush(stdout);
  return result;
}

//for local test
int main(int argc, char* argv[]) {
  // char const *a[]={"string1","string2"};
  // std::cout << *a <<std::endl;
  // std::cout << *(a+1) <<std::endl;
  // // return 0;
  // datadog::opentracing::TracerOptions tracer_options{"localhost", 8126, "jnr-sample"};
  // auto tracer = datadog::opentracing::makeTracer(tracer_options);
  //   {
  //       std::unordered_map<std::string, std::string> text_map = {{"x-datadog-parent-id", "5016006101185510646"},{"x-datadog-sampling-priority","1"},{"x-datadog-trace-id","6972236470085569872"}} ;
  //       std::unordered_map<std::string, std::string> cp;

  //       CustomCarrierReader carrierreader(text_map);
  //       auto span_context_maybe = tracer->Extract(carrierreader);
  //       if (!span_context_maybe) {
  //             // Extraction failed, log an error message.
  //             std::cerr << span_context_maybe.error().message() << "\n";
  //           }
  //       std::unique_ptr<opentracing::SpanContext> span_context =
  //               std::move(*span_context_maybe);

  //       //assert(!err);
  //       //assert(err.error() == span_context_corrupted_error);
  //           // How to get a readable message from the error.
  //       //std::cout << "Example error message: \"" << err.error().message() << "\"\n";
  //       auto span_cp = tracer->StartSpan("me", {opentracing::ChildOf(span_context_maybe->get())});
  //       int result=100;
  //       printf("result=%d\n", result);
  //       auto span_b = tracer->StartSpan("son", {opentracing::ChildOf(&span_cp->context())});
  //       span_b->SetTag("tag", "value");
  //   }
  //   tracer->Close();
    return 0;
}



