/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

// Don't modify this, instead set services.push.debug.
let gDebuggingEnabled = false;

function debug(s) {
  if (gDebuggingEnabled)
    dump("-*- Simplepush.js: " + s + "\n");
}

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/DOMRequestHelper.jsm");
Cu.import("resource://gre/modules/AppsUtils.jsm");

const PUSH_CID = Components.ID("{cde1d019-fad8-4044-b141-65fb4fb7a245}");

/**
 * The Push component runs in the child process and exposes the SimplePush API
 * to the web application. The PushService running in the parent process is the
 * one actually performing all operations.
 */
function Simplepush() {
  debug("Simplepush Constructor");
}

Simplepush.prototype = {
  __proto__: DOMRequestIpcHelper.prototype,

  contractID: "@mozilla.org/simplepush/SimplepushManager;1",

  classID : PUSH_CID,

  QueryInterface : XPCOMUtils.generateQI([Ci.nsIDOMGlobalPropertyInitializer,
                                          Ci.nsISupportsWeakReference,
                                          Ci.nsIObserver]),

  init: function(aWindow) {
    // Set debug first so that all debugging actually works.
    // NOTE: We don't add an observer here like in PushService. Flipping the
    // pref will require a reload of the app/page, which seems acceptable.
    gDebuggingEnabled = Services.prefs.getBoolPref("services.push.debug");
    debug("init()");

    let principal = aWindow.document.nodePrincipal;
    let appsService = Cc["@mozilla.org/AppsService;1"]
                        .getService(Ci.nsIAppsService);

    this._manifestURL = appsService.getManifestURLByLocalId(principal.appId);
    this._pageURL = principal.URI;

    this.initDOMRequestHelper(aWindow, [
      "SimplepushService:Register:OK",
      "SimplepushService:Register:KO",
      "SimplepushService:Unregister:OK",
      "SimplepushService:Unregister:KO",
      "SimplepushService:Registrations:OK",
      "SimplepushService:Registrations:KO"
    ]);

    this._cpmm = Cc["@mozilla.org/childprocessmessagemanager;1"]
                   .getService(Ci.nsISyncMessageSender);
  },

  receiveMessage: function(aMessage) {
    debug("receiveMessage()");
    let request = this.getRequest(aMessage.data.requestID);
    let json = aMessage.data;
    if (!request) {
      debug("No request " + json.requestID);
      return;
    }

    switch (aMessage.name) {
      case "SimplepushService:Register:OK":
        Services.DOMRequest.fireSuccess(request, json.pushEndpoint);
        break;
      case "SimplepushService:Register:KO":
        Services.DOMRequest.fireError(request, json.error);
        break;
      case "SimplepushService:Unregister:OK":
        Services.DOMRequest.fireSuccess(request, json.pushEndpoint);
        break;
      case "SimplepushService:Unregister:KO":
        Services.DOMRequest.fireError(request, json.error);
        break;
      case "SimplepushService:Registrations:OK":
        Services.DOMRequest.fireSuccess(request, json.registrations);
        break;
      case "SimplepushService:Registrations:KO":
        Services.DOMRequest.fireError(request, json.error);
        break;
      default:
        debug("NOT IMPLEMENTED! receiveMessage for " + aMessage.name);
    }
  },

  register: function() {
    debug("register()");
    let req = this.createRequest();
    if (!Services.prefs.getBoolPref("services.push.connection.enabled")) {
      // If push socket is disabled by the user, immediately error rather than
      // timing out.
      Services.DOMRequest.fireErrorAsync(req, "NetworkError");
      return req;
    }

    this._cpmm.sendAsyncMessage("Simplepush:Register", {
                                  pageURL: this._pageURL.spec,
                                  manifestURL: this._manifestURL,
                                  requestID: this.getRequestId(req)
                                });
    return req;
  },

  unregister: function(aPushEndpoint) {
    debug("unregister(" + aPushEndpoint + ")");
    let req = this.createRequest();
    this._cpmm.sendAsyncMessage("Simplepush:Unregister", {
                                  pageURL: this._pageURL.spec,
                                  manifestURL: this._manifestURL,
                                  requestID: this.getRequestId(req),
                                  pushEndpoint: aPushEndpoint
                                });
    return req;
  },

  registrations: function() {
    debug("registrations()");
    let req = this.createRequest();
    this._cpmm.sendAsyncMessage("Simplepush:Registrations", {
                                  manifestURL: this._manifestURL,
                                  requestID: this.getRequestId(req)
                                });
    return req;
  }
}

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([Simplepush]);
