BBHotpotato
===========

Hotpotato is a rich wrapper for **libcurl** written in Objective-C. Its name derives from the not-so-common misspelling of HTTP as HPTT.

It is an ARC-only library that uses [features](http://clang.llvm.org/docs/ObjectiveCLiterals.html) introduced by Clang 3.1. Thus, it is only suitable for iOS 5+ and OSX 10.7+.

If boasts an extremely simple and compact interface that allows you to reduce your code to fire off HTTP requests down to a couple of clean lines, while preserving full flexibility should you ever need it.

```objc
[[BBHTTPRequest getFrom:@"http://biasedbit.com"] execute:^(BBHTTPResponse* r) {
     NSLog(@"Finished: %u %@ -- received %u bytes of '%@'.",
           r.code, r.message, [r.data length], r[@"Content-Type"]);
 } error:^(NSError* e) {
     NSLog(@"Request failed: %@", [e localizedDescription]);
 }];

// Finished: 200 OK -- received 68364 bytes of 'text/html'.
```

> **IMPORTANT NOTE:**  
> SSL uploads are currently broken for content above ~1MB due to a bug in curl. I [reported it](http://curl.haxx.se/mail/lib-2013-01/0295.html) and Nick Zitzmann got me a patch that fixes it. You'll have to [build your own curl](https://github.com/brunodecarvalho/curl-ios-build-scripts) with that patch &mdash; it's the attachment on Nick's reply to my original email. [Ping me](https://twitter.com/biasedbit) if you want the patched static libs or need help patching curl.


At this stage there are probably things broken, rough edges to polish and features missing &mdash; using curl's multi handles and multipart uploads to name a few &mdash; to bring it up-to-par with other similar projects. I want to add those over time but help is always more than welcome so be sure to open issues for the features you'd love to see or drop me a mention [@biasedbit](http://twitter.com/biasedbit) on Twitter.


## Highlights

* Concise asynchronous-driven usage:

    ```objc
    [[BBHTTPRequest getFrom:@"http://biasedbit.com"] execute:^(BBHTTPResponse* response) {
        // handle response
    } error:nil]];
    ```

    > You don't even need to keep references to the requests, just fire and forget.


* Handy common usage patterns

    ```objc
    [[BBHTTPRequest getFrom:@"http://biasedbit.com"] setup:^(id request) {
        // Prepare request...
    } execute:^(BBHTTPResponse* response) {
        // Handle response...
    } error:^(NSError* error) {
        // Handle error...
    } finally:^{
        // Do after error OR success.
    }];


* Get JSON effortlessly

    ```objc
    [[BBJSONRequest getFrom:@"http://foo.bar"] getJSON:^(id result) {
        NSLog(@"User email: %@", result[@"user.email"]);
        NSLog(@"# of followers: %@", result[@"user.followers.@count"]);
    } error:^(NSError* error) {
        // Handle request *or* JSON decoding error
    }];
    ```

    > Notice the keyed subscript operator behaves as `valueForKeyPath:` rather than `valueForKey:`. That's because JSON responses that would yield a `NSDictionary` get wrapped by `BBJSONDictionary`.
    > Read more about the collection operators [here](http://developer.apple.com/library/mac/#documentation/Cocoa/Conceptual/KeyValueCoding/Articles/CollectionOperators.html);


* Stream uploads from a `NSInputStream` or directly from a file:

    ```objc
    [[BBHTTPRequest postFile:@"/path/to/file" to:@"http://api.target.url/"]
     setup:^(BBHTTPRequest* request) {
         request[@"Extra-Header"] = @"something else";
     } andExecute:^(BBHTTPResponse* response) {
         // handle response
     } error:nil];
    ```

    > The request's content type and content length headers will be automatically set based on the file's properties.


* Download to memory buffers or stream directly to file/`NSOutputStream`:

    ```objc
    [[BBHTTPRequest getFrom:@"http://biasedbit.com"]
     setup:^(BBHTTPRequest* request) {
         request.downloadToFile = @"/path/to/file";
     } andExecute:^(BBHTTPResponse* response) {
         // handle response
     } error:nil];
    ```

    > No need to delete the file if the download fails midway; hotpotato will take care of keeping everything clean.


* Even the *power-dev* API is clean and concise:

    ```objc
    BBHTTPExecutor* twitterExecutor = [BBHTTPExecutor initWithId:@"twitter.com"];
    BBHTTPExecutor* facebookExecutor = [BBHTTPExecutor initWithId:@"facebook.com"];
    twitterExecutor.maxParallelRequests = 10;
    facebookExecutor.maxParallelRequests = 2;
    ...
    BBHTTPRequest* request = [[BBHTTPRequest alloc]
                              initWithURL:[NSURL URLWithString:@"http://twitter.com"]
                              andVerb:@"GET"];

    request[@"Accept-Language"] = @"en-us";
    request.downloadProgressBlock = ^(NSUInteger current, NSUInteger total) { /* ... */ };
    request.finishBlock = ^(BBHTTPResponse* response) { /* ... */ };

    [twitterExecutor executeRequest:request];
    ```


## TODO list

* Multipart upload helpers
* Request queue
* Use curl's multi handles
* *Your bright idea here*


## Why?

You mean other than its sleek API or the fact that it uses libcurl underneath?

Well, unlike `NSURLConnection` and, consequently, any lib that relies on it, hotpotato...

* is strictly compliant with [section 8.2.3](http://tools.ietf.org/html/rfc2616#section-8.2.3) of RFC 2616, a.k.a. the misbeloved `Expect: 100-Continue` header;
* can receive server error responses midway through upload &mdash; as opposed to continuing to pump data into socket eden, and eventually reporting connection timeout instead of the actual error response sent by the server.

*"But my uploads work just fine..."*

* If you only wrote code that uploads to a server, you've probably never noticed either of the above;
* If you wrote both client *and* server-side code to handle uploads, chances are that you never ran into either of the above either;
* If you're hardcore and wrote your own server *and* client *and* noticed `NSURLConnection` ignores errors until it finishes its upload, then this is the HTTP framework for you. Also, fistbump for writing your server and client. And paying attention to the specs.

On a more serious tone, the motivation for this libcurl wrapper was that during development of [Droplr](http://droplr.com)'s API server, we noticed that whenever the API rejected an upload and immediately closed the connection &mdash; which is a perfectly legal & reasonable behavior &mdash; the Cocoa-based clients would keep reporting upload progress (even though I **knew** the socket was closed) and eventually fail with "Request timeout", instead of the response the server had sent down the pipes.

This meant that:

1. `NSURLConnection` wasn't waiting for the `100-Continue` provisional response before sending along the request body;
2. `NSURLConnection` wasn't realizing that a response was already sent and the connection was dying until it finished uploading what it had to upload. *stubborn bastard, eh?*

I did file a bug report but after a year of waiting for a response, I decided to come up with a working alternative. Coincidentally, the same day I let this library out in the open, I got a reply from Apple &mdash; closing the bug as a duplicate of some other I don't have access to.

A couple of quick tests with command line version of curl proved that curl knew how to properly handle these edge cases so it was time to build a new HTTP framework for Cocoa.

> During that process, [this handy build script](https://github.com/brunodecarvalho/curl-ios-build-scripts) was produced, so even if you don't want to use this library but are still interested in getting curl running on iOS, do check it out!


## Dependencies

* `libcurl`
* `libz.dylib`
* `Security.framework`
* `CoreServices.framework` on OSX
* `MobileCoreServices.framework` on iOS

> **Note:**  
> You can find libcurl binaries and headers under `Build/iOS/Static lib/libcurl` and `Build/OSX/Static lib/libcurl`. The iOS version was compiled against 6.0 SDK with support for i386 (simulator), armv7 and armv7s (iPhone 3GS and newer). The OSX version was compiled against 10.8 SDK with support for x86_64 (64 bit Intel). If you'd like to build your own custom version, try [this](https://github.com/brunodecarvalho/curl-ios-build-scripts).


## Documentation

For guides on how to setup and start working with this lib, check out [the wiki pages](https://github.com/brunodecarvalho/BBHotpotato/wiki).

The project also includes comprehensive class-level documentation. If you happen to have [appledoc](https://github.com/tomaz/appledoc) installed, just run the `generate` script on the `Docs` folder and it'll create html documentation for you under `Docs/html`.


## Credits

* Daniel Stenberg and everyone else involved in making cURL and libcurl
* Ben Copsey for the fantastic ASIHTTPRequest, which has been my HTTP workhorse on iOS since day 0


## License

Hotpotato is licensed under the Apache Software License version 2.0
