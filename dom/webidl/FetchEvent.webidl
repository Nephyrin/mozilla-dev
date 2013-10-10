/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The origin of this IDL file is
 * http://slightlyoff.github.io/ServiceWorker/spec/service_worker/index.html
 *
 * Copyright © 2012 W3C® (MIT, ERCIM, Keio), All Rights Reserved. W3C
 * liability, trademark and document use rules apply.
 */

enum RequestType {
  "navigate",
  "fetch"
};

[Constructor(DOMString type, optional FetchEventInit eventInitDict)]
interface FetchEvent : Event {
  readonly attribute Request request;
  readonly attribute RequestType type;
  readonly attribute boolean isTopLevel;
  readonly attribute boolean isReload;

  void respondWith(Promise r);
  void respondWith(Response r);
  Promise forwardTo(URL url);
  Promise forwardTo(DOMString url);
};

dictionary FetchEventInit : EventInit {
  Request? request = null;
  RequestType type = "navigate";
  boolean isTopLevel = false;
  boolean isReload = false;
};
