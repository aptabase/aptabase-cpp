![Aptabase](https://aptabase.com/og.png)

# C++ SDK for Aptabase

## Design Goals

The Aptabase Client SDK for C++ was designed with versatility and flexibility as core principles, enabling integration across a wide spectrum of applications, from video games to web servers. This chapter outlines the design goals that guided the development of this SDK.

### Adaptability to Various Execution Models

One of the primary design goals was to support different execution models, accounting for the diverse ways applications handle timing and processing:

- **Application-Loop Ticking**: Support for applications that operate on a frame-by-frame basis, where the analytics events can be transported during the loop
- **Thread-based Processing**: Compatibility with systems that prefer to allocate dedicated threads for analytics operations
- **Boost-like Asynchronous Implementation**: Support for applications using asynchronous programming patterns similar to Boost.Asio, allowing for non-blocking operations (Work in progress)

### Abstraction of Networking Implementation

C++ offers numerous approaches to handling HTTP requests, ranging from platform-specific APIs to third-party libraries. To accommodate this diversity, we:

- Implemented an abstract interface for HTTP operations, allowing different HTTP client implementations to be plugged in
- Provided default implementations for cpp-httplib and boost, while making it easy to substitute custom networking code

## Integration

Aptabase-cpp uses CMake as it's build system. It's user's responsibility to supply all dependencies. Also supplied networking implementation does not compile by default. 

CMake example with boost enabled

```cmake
# Code to include nlohmann_json, boost and openssl dependencies
# Based on your setup, it may be add_subdirectory or find_package

set(CMAKE_APTABASE_USE_BOOST ON)
add_subdirectory(path/to/aptabase-cpp)
```

CMake example with cpp-httplib enabled

```cmake
# Code to include nlohmann_json, cpp-httplib and openssl dependencies
# Based on your setup, it may be add_subdirectory or find_package

set(CMAKE_APTABASE_USE_HTTPLIB ON)
add_subdirectory(path/to/aptabase-cpp)
```

## Examples

Here is minimal aptabase-cpp example, required to submit your first event to analytics system

```c++
#include <aptabase/analytics.hpp>
#include <aptabase/provider/worker_provider.hpp>
#include <aptabase/net/httplib.hpp>

int main(){
    std::string aptabase_app_key = "<Your app key>";
    std::string aptabase_app_url = "https://your.aptabase.url"
    bool aptabase_is_debug = true;

    Aptabase::Analytics aptabase(
        std::make_unique<Aptabase::WorkerProvider>(
			std::make_unique<Aptabase::HttplibHttpClient>(),
			aptabase_app_key,
			aptabase_app_url
		), 
		aptabase_is_debug
    );

    aptabase.StartSession();
	aptabase.RecordEvent("TestEvent1");
    aptabase.EndSession();
}

```

Event may have additional attributes of std::string, float or double types

```c++

aptabase.RecordEvent("TestEvent1", {{"Attribute1", 0.f}, {"Attribute2", 234.0}, {"Attribute3", "Value"}});

```

By default, unique sessionId is generated each StartSession call.
Custom sessionId also can be provided.

```c++
std::string customSessionId = "12345";

aptabase.StartSession(customSessionId);
aptabase.RecordEvent("TestEvent1");
aptabase.EndSession();
```

System attributes can be supplied and/or changed in runtime.

```c++

aptabase.SetLocale("en-US");
aptabase.SetAppVersion("0.0.1");
aptabase.SetOsName("Linux");
aptabase.SetOsVersion("5.17");

aptabase.SetDebug(true);
aptabase.RecordEvent("TestEvent1");
aptabase.SetDebug(false);
aptabase.RecordEvent("TestEvent1");
```
