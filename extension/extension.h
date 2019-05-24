/**
 * vim: set ts=4 :
 * =============================================================================
 * SendVar Proxy Manager
 * Copyright (C) 2011-2019 Afronanny & AlliedModders community.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _EXTENSION_H_INC_
#define _EXTENSION_H_INC_

 /*
	TODO:
		Implement interface for prop change hooks & add natives
		Allow multiple hooks for prop on same entity (probably use delegate for this?)
 */

#include "smsdk_ext.h"
#include <string>
#include <stdint.h>
#include "convar.h"
#include "dt_send.h"
#include "server_class.h"
#include "ISendProxy.h"
#include <eiface.h>
#include <ISDKHooks.h>
#include <ISDKTools.h>

#define GET_CONVAR(name) \
	name = g_pCVar->FindVar(#name); \
	if (name == nullptr) { \
		if (error != nullptr && maxlen != 0) { \
			ismm->Format(error, maxlen, "Could not find ConVar: " #name); \
		} \
		return false; \
	}

class IServerGameEnts;

void GlobalProxy(const SendProp *pProp, const void *pStructBase, const void* pData, DVariant *pOut, int iElement, int objectID);
void GlobalProxyGamerules(const SendProp *pProp, const void *pStructBase, const void* pData, DVariant *pOut, int iElement, int objectID);
bool IsPropValid(SendProp *, PropType);

struct ListenerCallbackInfo
{
	IExtension *							m_pExt;
	IExtensionInterface *					m_pExtAPI;
	ISendProxyUnhookListener *				m_pCallBack;
};

struct SendPropHook
{
	SendPropHook() { vListeners = new CUtlVector<ListenerCallbackInfo>(); }
	SendPropHook(const SendPropHook & rObj)
	{
		memcpy(this, &rObj, sizeof(SendPropHook));
		vListeners = new CUtlVector<ListenerCallbackInfo>();
		*vListeners = *rObj.vListeners;
	}
	~SendPropHook() { delete vListeners; }
	void *									pCallback;
	CallBackType							iCallbackType;
	SendProp *								pVar;
	edict_t *								pEnt;
	SendVarProxyFn							pRealProxy;
	int										objectID;
	PropType								PropType;
	int										Offset;
	int										Element{0};
	IExtensionInterface *					pExtensionAPI{nullptr};
	CUtlVector<ListenerCallbackInfo> *		vListeners;
};

struct SendPropHookGamerules
{
	SendPropHookGamerules() { vListeners = new CUtlVector<ListenerCallbackInfo>(); }
	SendPropHookGamerules(const SendPropHookGamerules & rObj)
	{
		memcpy(this, &rObj, sizeof(SendPropHookGamerules));
		vListeners = new CUtlVector<ListenerCallbackInfo>();
		*vListeners = *rObj.vListeners;
	}
	~SendPropHookGamerules() { delete vListeners; }
	void *									pCallback;
	CallBackType							iCallbackType;
	SendProp *								pVar;
	SendVarProxyFn							pRealProxy;
	PropType								PropType;
	int										Offset;
	int										Element{0};
	IExtensionInterface *					pExtensionAPI{nullptr};
	CUtlVector<ListenerCallbackInfo> *		vListeners;
};

struct PropChangeHook
{
	IPluginFunction *						pCallback;
	int										iLastValue;
	float									flLastValue;
	char									cLastValue[4096];
	SendProp *								pVar;
	PropType								PropType;
	unsigned int							Offset;
	int										objectID;
};

struct PropChangeHookGamerules
{
	IPluginFunction *						pCallback;
	int										iLastValue;
	float									flLastValue;
	char									cLastValue[4096];
	SendProp *								pVar;
	PropType								PropType;
	unsigned int							Offset;
};
 
class SendProxyManager :
	public SDKExtension,
	public IPluginsListener,
	public ISMEntityListener
{
public: //sm
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();
	virtual void SDK_OnAllLoaded();
	
	virtual void OnCoreMapEnd();
	virtual void OnCoreMapStart(edict_t *, int, int);
	//virtual void SDK_OnPauseChange(bool paused);

	//virtual bool QueryRunning(char *error, size_t maxlength);
public: //other
	void OnPluginUnloaded(IPlugin *plugin);
	//Returns true upon success
	//Returns false if hook exists for that object and prop
	//Returns false if the prop does not exist or the edict does not exist/is free
	bool AddHookToList(SendPropHook hook);
	bool AddHookToListGamerules(SendPropHookGamerules hook);

	void UnhookProxy(int i);
	void UnhookProxyGamerules(int i);
	virtual int GetClientCount() const;
public: // ISMEntityListener
	virtual void OnEntityDestroyed(CBaseEntity *pEntity);
public:
#if defined SMEXT_CONF_METAMOD
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	//virtual bool SDK_OnMetamodUnload(char *error, size_t maxlength);
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength);
#endif
};

extern SendProxyManager g_SendProxyManager;
extern IServerGameEnts * gameents;
extern CUtlVector<SendPropHook> g_Hooks;
extern CUtlVector<SendPropHookGamerules> g_HooksGamerules;
extern CUtlVector<PropChangeHook> g_ChangeHooks;
extern CUtlVector<PropChangeHookGamerules> g_ChangeHooksGamerules;
extern const char * g_szGameRulesProxy;
extern int g_iEdictCount;
extern ISDKTools * g_pSDKTools;

#endif // _EXTENSION_H_INC_
