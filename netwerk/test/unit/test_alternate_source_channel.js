"use strict";
// https://bugzilla.mozilla.org/show_bug.cgi?id=761228

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;
const Cr = Components.results;

Cu.import("resource://testing-common/httpd.js");

XPCOMUtils.defineLazyGetter(this, "URL", function() {
  return "http://localhost:" + httpServer.identity.primaryPort;
});

var httpServer = null;

function make_uri(url) {
  var ios = Cc["@mozilla.org/network/io-service;1"].
            getService(Ci.nsIIOService);
  return ios.newURI(url, null, null);
}

function make_channel(url) {
  var ios = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
  var chan = ios.newChannel(url, null, null).QueryInterface(Ci.nsIHttpChannel);
  return chan;
}

const REMOTE_BODY = "http handler body";
const NON_REMOTE_BODY = "synthesized body";
const NON_REMOTE_RESPONSE = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + NON_REMOTE_BODY.length + "\r\n\r\n" + NON_REMOTE_BODY;

function bodyHandler(metadata, response) {
  response.setHeader('Content-Type', 'text/plain');
  response.write(REMOTE_BODY);
}

function run_test() {
  httpServer = new HttpServer();
  httpServer.registerPathHandler('/body', bodyHandler);
  httpServer.start(-1);

  run_next_test();
}

add_test(function() {
  var chan = make_channel(URL + '/body');
  var ios = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
  chan = ios.QueryInterface(Ci.nsINetUtil).createAlternateSourceChannel(chan,
           function() {
  var synthesized = Cc["@mozilla.org/io/string-input-stream;1"]
                      .createInstance(Ci.nsIStringInputStream);
  synthesized.data = NON_REMOTE_RESPONSE;
  chan.initiateAlternateResponse(synthesized);
           });
  chan.QueryInterface(Ci.nsIAlternateSourceChannel);
  chan.asyncOpen(new ChannelListener(handle_synthesized_response, null), null);
});

function handle_synthesized_response(request, buffer) {
  do_check_eq(buffer, NON_REMOTE_BODY);
  run_next_test();
}

add_test(function() {
  var chan = make_channel(URL + '/body');
  var ios = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
  chan = ios.QueryInterface(Ci.nsINetUtil).createAlternateSourceChannel(chan,
           function() {
             do_execute_soon(function() { chan.forwardToOriginalChannel(); });
           });
  chan.QueryInterface(Ci.nsIAlternateSourceChannel);
  chan.asyncOpen(new ChannelListener(handle_remote_response, null), null);
});

function handle_remote_response(request, buffer) {
  do_check_eq(buffer, REMOTE_BODY);
  httpServer.stop(run_next_test);
}
