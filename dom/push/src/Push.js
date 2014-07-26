/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

// Don't modify this, instead set services.push.debug.
let gDebuggingEnabled = false;

function debug(s) {
  if (gDebuggingEnabled)
    dump("-*- Push.js: " + s + "\n");
}

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cu = Components.utils;

Cu.import("resource://gre/modules/XPCOMUtils.jsm");
Cu.import("resource://gre/modules/Services.jsm");
Cu.import("resource://gre/modules/DOMRequestHelper.jsm");
Cu.import("resource://gre/modules/AppsUtils.jsm");
Cu.import("resource://gre/modules/PrivateBrowsingUtils.jsm");

const PUSH_CID = Components.ID("{cde1d019-beef-4044-b141-65fb4fb7a245}");

/**
 * The Push component runs in the child process and exposes the SimplePush API
 * to the web application. The PushService running in the parent process is the
 * one actually performing all operations.
 */
function Push() {
  debug("Push Constructor");
}

Push.prototype = {
  __proto__: DOMRequestIpcHelper.prototype,

  contractID: "@mozilla.org/push/PushRegistrationManager;1",

  classID : PUSH_CID,

  QueryInterface : XPCOMUtils.generateQI([Ci.nsIDOMGlobalPropertyInitializer,
                                          Ci.nsISupportsWeakReference,
                                          Ci.nsIObserver]),

  init: function(aWindow) {
    // Set debug first so that all debugging actually works.
    // NOTE: We don't add an observer here like in PushService. Flipping the
    // pref will require a reload of the app/page, which seems acceptable.
    gDebuggingEnabled = Services.prefs.getBoolPref("dom.push.debug");
    debug("init()");

    if (PrivateBrowsingUtils.isWindowPrivate(aWindow)) {
      debug("Push is disabled in private browsing mode.");
      return null;
    }

    let appsService = Cc["@mozilla.org/AppsService;1"]
                        .getService(Ci.nsIAppsService);

    let principal = this._principal = aWindow.document.nodePrincipal;

    this._manifestURL = appsService.getManifestURLByLocalId(principal.appId);
    this._pageURI = principal.URI;

    // If its an app, test for permission.
    if (principal.appId != Ci.nsIScriptSecurityManager.NO_APP_ID) {
      this._manifestURL = appsService.getManifestURLByLocalId(principal.appId);

      let perm = Services.perms.testExactPermissionFromPrincipal(principal,
                                                                 "push");
      if (perm != Ci.nsIPermissionManager.ALLOW_ACTION) {
        debug("App does not have push permission");
        return null;
      }
    } else {
      // If we are in a non-URI principal like the system principal there is
      // no origin or manifest.
      if (!principal.URI) {
        throw Components.results.NS_ERROR_FAILURE;
      }

      this._callerIsWebPage = true;
    }



    this.initDOMRequestHelper(aWindow, [
      "PushService:Register:OK",
      "PushService:Register:KO",
      "PushService:Unregister:OK",
      "PushService:Unregister:KO",
      "PushService:IsRegistered:OK",
      "PushService:IsRegistered:KO"
    ]);

    this._cpmm = Cc["@mozilla.org/childprocessmessagemanager;1"]
                   .getService(Ci.nsISyncMessageSender);
  },

  receiveMessage: function(aMessage) {
    debug("receiveMessage()");
    let resolver = this.takePromiseResolver(aMessage.data.resolverID);
    let json = aMessage.data;

    switch (aMessage.name) {
      case "PushService:Register:OK":
        resolver.resolve(json.pushEndpoint);
        break;
      case "PushService:Register:KO":
        resolver.reject(json.error);
        break;
      case "PushService:Unregister:OK":
        resolver.resolve(true)
        break;
      case "PushService:Unregister:KO":
        resolver.resolve(false) 
        break;
      case "PushService:IsRegistered:OK":
        resolver.resolve(json.isRegistered);
        break;
      case "PushService:IsRegistered:KO":
        resolver.reject(json.error);
        break;
      default:
        debug("NOT IMPLEMENTED! receiveMessage for " + aMessage.name);
    }
  },

  //aOptions is unused for now
  register: function(aOptions) {
    debug("register()");
    let promise = this.createPromise((resolve, reject) => {

      if (!Services.prefs.getBoolPref("dom.push.enabled")) {
        // If push socket is disabled by the user, immediately error rather than
        // timing out.
        reject('NetworkError');
        return;
      }

      let resolverID = this.getPromiseResolverId({resolve:resolve, reject:reject})
      let wakeupURI = this._pageURI;
      if (aOptions && aOptions.page) {
        try {
          wakeupURI = Services.io.newURI(aOptions.page, null, this._pageURI);
        } catch (e) {
          debug(e.message);
          throw Components.results.NS_ERROR_INVALID_ARG;
        }

        Services.scriptSecurityManager.checkSameOriginURI(this._pageURI,
                                                          wakeupURI,
                                                          true);
      }


      let permPrompt = Cc["@mozilla.org/content-permission/prompt;1"]
                         .createInstance(Ci.nsIContentPermissionPrompt);

      if (!permPrompt) {
        reject("SecurityError");
        return;
      }

      let types = Cc["@mozilla.org/array;1"].createInstance(Ci.nsIMutableArray);
      let promptType = {
        type: 'push',
        access: "unused",
        QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPermissionType])
      };
      types.appendElement(promptType, false);

      permPrompt.prompt({
        types: types,
        principal: this._principal,
        window: this._window,
        cancel: function() {
          reject("Security Error.");
        },
        allow: function() {
          this._cpmm.sendAsyncMessage("Push:Register", {
                                        pageURL:    this._pageURI.spec,
                                        wakeupURL:  wakeupURI.spec,
                                        wakeupType: 'page',
                                        resolverID: resolverID
                                      });
        }.bind(this)
      });

    });

    return promise;
  },

  unregister: function() {
    debug("unregister()");

    let promise = this.createPromise((resolve, reject) => {
      let resolverID = this.getPromiseResolverId({resolve:resolve, reject:reject});

      this._cpmm.sendAsyncMessage("Push:Unregister", {
                                    pageURL: this._pageURI.spec,
                                    manifestURL: this._manifestURL,
                                    resolverID:resolverID
                                  });
      }
    );

    return promise;
  },

  isRegistered: function() {
    debug("isRegistered()");

    let promise = this.createPromise((resolve, reject) => {
      let resolverID = this.getPromiseResolverId({resolve:resolve, reject:reject})

      this._cpmm.sendAsyncMessage("Push:IsRegistered", {
                        pageURL: this._pageURI.spec,
                        manifestURL: this._manifestURL,
                        resolverID:  resolverID
                      });
      }
    );
    return promise;

  },

  hasPermission: function() {
    debug("hasPermission()");
    let promise = this.createPromise((resolve, reject) => {
      let result = Services.perms.testExactPermissionFromPrincipal(this._principal, "push");
      switch(result){
        case Ci.nsIPermissionManager.ALLOW_ACTION: resolve('granted');
          break;
        case Ci.nsIPermissionManager.DENY_ACTION: resolve('denied');
          break;
        case Ci.nsIPermissionManager.UNKNOWN_ACTION: resolve('needask');
          break;
        default: reject('derp');
      }
    });
    return promise;
  }
}

this.NSGetFactory = XPCOMUtils.generateNSGetFactory([Push]);
