/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * The origin of this IDL file is
 * http://fetch.spec.whatwg.org/
 */

typedef object JSON;
// FIXME: Bug 1025183 ScalarValueString.
typedef (ArrayBuffer or ArrayBufferView or Blob or DOMString) FetchBodyInit;
// FIXME(nsm): JSON support
typedef FetchBodyInit FetchBody;

[NoInterfaceObject]
interface FetchBodyStream {
  // Promise<ArrayBuffer>
  Promise asArrayBuffer();
  // Promise<Blob>
  Promise asBlob();
  // Promise<FormData>
  Promise asFormData();
  // Promise<JSON>
  Promise asJSON();
  // Promise<ScalarValueString>
  Promise asText();
};
