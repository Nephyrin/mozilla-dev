/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The origin of this IDL file is
 * http://fetch.spec.whatwg.org/
 */

// FIXME: Bug 1025183 typedef (Request or ScalarValueString) RequestInfo;
typedef (Request or DOMString) RequestInfo;

[Constructor(RequestInfo input, optional RequestInit init),
 // FIXME Exposed=Window,Worker
 Pref="dom.fetch.enabled"]
interface Request {
  readonly attribute ByteString method;
  // FIXME: Bug 1025183 attribute ScalarValueString url;
  readonly attribute DOMString url;
  readonly attribute Headers headers;

  readonly attribute FetchBodyStream body;

  readonly attribute DOMString referrer;
  readonly attribute RequestMode mode;
  readonly attribute RequestCredentials credentials;
};

dictionary RequestInit {
  ByteString method;
  HeadersInit headers;
  FetchBodyInit body;
  RequestMode mode;
  RequestCredentials credentials;
};

enum RequestMode { "same-origin", "no-cors", "cors" };
enum RequestCredentials { "omit", "same-origin", "include" };
