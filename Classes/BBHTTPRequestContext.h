//
// Copyright 2013 BiasedBit
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//
//  Created by Bruno de Carvalho (@biasedbit, http://biasedbit.com)
//  Copyright (c) 2013 BiasedBit. All rights reserved.
//

#import "BBHTTPRequest.h"
#import "curl.h"



#pragma mark - Enums

typedef NS_ENUM(NSUInteger, BBHTTPResponseState) {
    BBHTTPResponseStateReadingStatusLine = 0,
    BBHTTPResponseStateReadingHeaders,
    BBHTTPResponseStateReadingData
};



#pragma mark -

/**
 The `BBHTTPRequestContext` class holds the originating request and all the responses received in its context.
 
 It mainly serves the purpose of hiding away some of the logic present in transitioning between states in the lifecycle
 of an HTTP request/response flow, as well as factoring out some of the logic that `<BBHTTPExecutor>` requires.
 As such, has no value outside of it.
 */
@interface BBHTTPRequestContext : NSObject


#pragma mark Creating a request context

/// --------------------------------
/// @name Creating a request context
/// --------------------------------

- (id)initWithRequest:(BBHTTPRequest*)request andCurlHandle:(CURL*)handle;


#pragma mark Managing state transitions

/// --------------------------------
/// @name Managing state transitions
/// --------------------------------

@property(assign, nonatomic) BBHTTPResponseState state;

- (BOOL)finishCurrentResponse;
- (void)cleanup:(BOOL)success;
- (void)finish;
- (void)finishWithError:(NSError*)error;
- (void)deleteFileInBackground:(NSString*)file;


#pragma mark Managing the upload

/// -------------------------
/// @name Managing the upload
/// -------------------------

@property(assign, nonatomic) BOOL uploadAccepted;
@property(assign, nonatomic) BOOL uploadPaused;
@property(assign, nonatomic, readonly) BOOL uploadAborted;
@property(assign, nonatomic, readonly) NSUInteger uploadedBytes;
@property(assign, nonatomic, readonly) NSUInteger downloadSize;
@property(assign, nonatomic, readonly) NSUInteger downloadedBytes;

- (BOOL)is100ContinueRequired;
- (NSInteger)transferInputToBuffer:(uint8_t*)buffer limit:(NSUInteger)limit;


#pragma mark Reading data from the server

/// ----------------------------------
/// @name Reading data from the server
/// ----------------------------------

- (BOOL)beginResponseWithLine:(NSString*)line;
- (BOOL)addHeaderToCurrentResponse:(NSString*)headerLine;
- (BOOL)appendDataToCurrentResponse:(uint8_t*)bytes withLength:(NSUInteger)length;


#pragma mark Querying context information

/// ----------------------------------
/// @name Querying context information
/// ----------------------------------

@property(strong, nonatomic, readonly) BBHTTPRequest* request;
@property(assign, nonatomic, readonly) CURL* handle;
@property(strong, nonatomic, readonly) NSMutableArray* receivedResponses;
@property(strong, nonatomic, readonly) NSError* error;
@property(strong, nonatomic, readonly) BBHTTPResponse* currentResponse;
@property(strong, nonatomic, readonly) BBHTTPResponse* lastResponse;

@end
