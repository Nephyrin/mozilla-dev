/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The origin of this IDL file is
 * http://fetch.spec.whatwg.org/
 */

[Constructor(optional FetchBodyInit body, optional ResponseInit init),
 // Exposed=Window,Worker,
 Pref="dom.fetch.enabled"]
interface Response {
  // FIXME(nsm): Bug 1025183 ScalarValueString url.
  static Response redirect(DOMString url, optional unsigned short status = 302);

  readonly attribute ResponseType type;

  // FIXME(nsm): Bug 1025183 readonly attribute ScalarValueString url;
  readonly attribute DOMString url;
  readonly attribute unsigned short status;
  readonly attribute ByteString statusText;
  readonly attribute Headers headers;
  readonly attribute FetchBodyStream body;
};

dictionary ResponseInit {
  unsigned short status = 200;
  // Becase we don't seem to support default values for ByteString.
  ByteString statusText; // = "OK";
  HeadersInit headers;
};

enum ResponseType { "basic", "cors", "default", "error", "opaque" };
