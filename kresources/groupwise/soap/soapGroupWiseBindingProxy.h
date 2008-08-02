/* soapGroupWiseBindingProxy.h
   Generated by gSOAP 2.7.3 from ./groupwise.h
   Copyright (C) 2000-2005, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapGroupWiseBinding_H
#define soapGroupWiseBinding_H
#include "soapH.h"
class GroupWiseBinding
{   public:
	struct soap *soap;
	const char *endpoint;
	GroupWiseBinding()
	{ soap = soap_new(); endpoint = "http://localhost:8080"; if (soap && !soap->namespaces) { static const struct Namespace namespaces[] = 
{
	{"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
	{"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
	{"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
	{"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
	{"ngwt", "http://schemas.novell.com/2005/01/GroupWise/types", NULL, NULL},
	{"ngwm", "http://schemas.novell.com/2005/01/GroupWise/methods", NULL, NULL},
	{"ngw", "http://schemas.novell.com/2005/01/GroupWise/groupwise.wsdl", NULL, NULL},
	{"ngwe", "http://schemas.novell.com/2005/01/GroupWise/events", NULL, NULL},
	{NULL, NULL, NULL, NULL}
};
	soap->namespaces = namespaces; } };
	virtual ~GroupWiseBinding() { if (soap) { soap_destroy(soap); soap_end(soap); soap_done(soap); free((void*)soap); } };
	virtual int __ngw__acceptRequest(_ngwm__acceptRequest *ngwm__acceptRequest, _ngwm__acceptResponse *ngwm__acceptResponse) { return soap ? soap_call___ngw__acceptRequest(soap, endpoint, NULL, ngwm__acceptRequest, ngwm__acceptResponse) : SOAP_EOM; };
	virtual int __ngw__acceptShareRequest(_ngwm__acceptShareRequest *ngwm__acceptShareRequest, _ngwm__acceptShareResponse *ngwm__acceptShareResponse) { return soap ? soap_call___ngw__acceptShareRequest(soap, endpoint, NULL, ngwm__acceptShareRequest, ngwm__acceptShareResponse) : SOAP_EOM; };
	virtual int __ngw__addItemRequest(_ngwm__addItemRequest *ngwm__addItemRequest, _ngwm__addItemResponse *ngwm__addItemResponse) { return soap ? soap_call___ngw__addItemRequest(soap, endpoint, NULL, ngwm__addItemRequest, ngwm__addItemResponse) : SOAP_EOM; };
	virtual int __ngw__addItemsRequest(_ngwm__addItemsRequest *ngwm__addItemsRequest, _ngwm__addItemsResponse *ngwm__addItemsResponse) { return soap ? soap_call___ngw__addItemsRequest(soap, endpoint, NULL, ngwm__addItemsRequest, ngwm__addItemsResponse) : SOAP_EOM; };
	virtual int __ngw__addMembersRequest(_ngwm__addMembersRequest *ngwm__addMembersRequest, _ngwm__addMembersResponse *ngwm__addMembersResponse) { return soap ? soap_call___ngw__addMembersRequest(soap, endpoint, NULL, ngwm__addMembersRequest, ngwm__addMembersResponse) : SOAP_EOM; };
	virtual int __ngw__closeFreeBusySessionRequest(_ngwm__closeFreeBusySessionRequest *ngwm__closeFreeBusySessionRequest, _ngwm__closeFreeBusySessionResponse *ngwm__closeFreeBusySessionResponse) { return soap ? soap_call___ngw__closeFreeBusySessionRequest(soap, endpoint, NULL, ngwm__closeFreeBusySessionRequest, ngwm__closeFreeBusySessionResponse) : SOAP_EOM; };
	virtual int __ngw__completeRequest(_ngwm__completeRequest *ngwm__completeRequest, _ngwm__completeResponse *ngwm__completeResponse) { return soap ? soap_call___ngw__completeRequest(soap, endpoint, NULL, ngwm__completeRequest, ngwm__completeResponse) : SOAP_EOM; };
	virtual int __ngw__createCursorRequest(_ngwm__createCursorRequest *ngwm__createCursorRequest, _ngwm__createCursorResponse *ngwm__createCursorResponse) { return soap ? soap_call___ngw__createCursorRequest(soap, endpoint, NULL, ngwm__createCursorRequest, ngwm__createCursorResponse) : SOAP_EOM; };
	virtual int __ngw__createItemRequest(_ngwm__createItemRequest *ngwm__createItemRequest, _ngwm__createItemResponse *ngwm__createItemResponse) { return soap ? soap_call___ngw__createItemRequest(soap, endpoint, NULL, ngwm__createItemRequest, ngwm__createItemResponse) : SOAP_EOM; };
	virtual int __ngw__createItemsRequest(_ngwm__createItemsRequest *ngwm__createItemsRequest, _ngwm__createItemsResponse *ngwm__createItemsResponse) { return soap ? soap_call___ngw__createItemsRequest(soap, endpoint, NULL, ngwm__createItemsRequest, ngwm__createItemsResponse) : SOAP_EOM; };
	virtual int __ngw__createJunkEntryRequest(_ngwm__createJunkEntryRequest *ngwm__createJunkEntryRequest, _ngwm__createJunkEntryResponse *ngwm__createJunkEntryResponse) { return soap ? soap_call___ngw__createJunkEntryRequest(soap, endpoint, NULL, ngwm__createJunkEntryRequest, ngwm__createJunkEntryResponse) : SOAP_EOM; };
	virtual int __ngw__createProxyAccessRequest(_ngwm__createProxyAccessRequest *ngwm__createProxyAccessRequest, _ngwm__createProxyAccessResponse *ngwm__createProxyAccessResponse) { return soap ? soap_call___ngw__createProxyAccessRequest(soap, endpoint, NULL, ngwm__createProxyAccessRequest, ngwm__createProxyAccessResponse) : SOAP_EOM; };
	virtual int __ngw__createSignatureRequest(_ngwm__createSignatureRequest *ngwm__createSignatureRequest, _ngwm__createSignatureResponse *ngwm__createSignatureResponse) { return soap ? soap_call___ngw__createSignatureRequest(soap, endpoint, NULL, ngwm__createSignatureRequest, ngwm__createSignatureResponse) : SOAP_EOM; };
	virtual int __ngw__declineRequest(_ngwm__declineRequest *ngwm__declineRequest, _ngwm__declineResponse *ngwm__declineResponse) { return soap ? soap_call___ngw__declineRequest(soap, endpoint, NULL, ngwm__declineRequest, ngwm__declineResponse) : SOAP_EOM; };
	virtual int __ngw__delegateRequest(_ngwm__delegateRequest *ngwm__delegateRequest, _ngwm__delegateResponse *ngwm__delegateResponse) { return soap ? soap_call___ngw__delegateRequest(soap, endpoint, NULL, ngwm__delegateRequest, ngwm__delegateResponse) : SOAP_EOM; };
	virtual int __ngw__destroyCursorRequest(_ngwm__destroyCursorRequest *ngwm__destroyCursorRequest, _ngwm__destroyCursorResponse *ngwm__destroyCursorResponse) { return soap ? soap_call___ngw__destroyCursorRequest(soap, endpoint, NULL, ngwm__destroyCursorRequest, ngwm__destroyCursorResponse) : SOAP_EOM; };
	virtual int __ngw__executeRuleRequest(_ngwm__executeRuleRequest *ngwm__executeRuleRequest, _ngwm__executeRuleResponse *ngwm__executeRuleResponse) { return soap ? soap_call___ngw__executeRuleRequest(soap, endpoint, NULL, ngwm__executeRuleRequest, ngwm__executeRuleResponse) : SOAP_EOM; };
	virtual int __ngw__forwardRequest(_ngwm__forwardRequest *ngwm__forwardRequest, _ngwm__forwardResponse *ngwm__forwardResponse) { return soap ? soap_call___ngw__forwardRequest(soap, endpoint, NULL, ngwm__forwardRequest, ngwm__forwardResponse) : SOAP_EOM; };
	virtual int __ngw__getAddressBookListRequest(_ngwm__getAddressBookListRequest *ngwm__getAddressBookListRequest, _ngwm__getAddressBookListResponse *ngwm__getAddressBookListResponse) { return soap ? soap_call___ngw__getAddressBookListRequest(soap, endpoint, NULL, ngwm__getAddressBookListRequest, ngwm__getAddressBookListResponse) : SOAP_EOM; };
	virtual int __ngw__getAttachmentRequestMessage(_ngwm__getAttachmentRequest *ngwm__getAttachmentRequest, _ngwm__getAttachmentResponse *ngwm__getAttachmentResponse) { return soap ? soap_call___ngw__getAttachmentRequestMessage(soap, endpoint, NULL, ngwm__getAttachmentRequest, ngwm__getAttachmentResponse) : SOAP_EOM; };
	virtual int __ngw__getCategoryListRequest(_ngwm__getCategoryListRequest *ngwm__getCategoryListRequest, _ngwm__getCategoryListResponse *ngwm__getCategoryListResponse) { return soap ? soap_call___ngw__getCategoryListRequest(soap, endpoint, NULL, ngwm__getCategoryListRequest, ngwm__getCategoryListResponse) : SOAP_EOM; };
	virtual int __ngw__getCustomListRequest(_ngwm__getCustomListRequest *ngwm__getCustomListRequest, _ngwm__getCustomListResponse *ngwm__getCustomListResponse) { return soap ? soap_call___ngw__getCustomListRequest(soap, endpoint, NULL, ngwm__getCustomListRequest, ngwm__getCustomListResponse) : SOAP_EOM; };
	virtual int __ngw__getDeltasRequest(_ngwm__getDeltasRequest *ngwm__getDeltasRequest, _ngwm__getDeltasResponse *ngwm__getDeltasResponse) { return soap ? soap_call___ngw__getDeltasRequest(soap, endpoint, NULL, ngwm__getDeltasRequest, ngwm__getDeltasResponse) : SOAP_EOM; };
	virtual int __ngw__getDeltaInfoRequest(_ngwm__getDeltaInfoRequest *ngwm__getDeltaInfoRequest, _ngwm__getDeltaInfoResponse *ngwm__getDeltaInfoResponse) { return soap ? soap_call___ngw__getDeltaInfoRequest(soap, endpoint, NULL, ngwm__getDeltaInfoRequest, ngwm__getDeltaInfoResponse) : SOAP_EOM; };
	virtual int __ngw__getDocumentTypeListRequest(_ngwm__getDocumentTypeListRequest *ngwm__getDocumentTypeListRequest, _ngwm__getDocumentTypeListResponse *ngwm__getDocumentTypeListResponse) { return soap ? soap_call___ngw__getDocumentTypeListRequest(soap, endpoint, NULL, ngwm__getDocumentTypeListRequest, ngwm__getDocumentTypeListResponse) : SOAP_EOM; };
	virtual int __ngw__getFolderRequest(_ngwm__getFolderRequest *ngwm__getFolderRequest, _ngwm__getFolderResponse *ngwm__getFolderResponse) { return soap ? soap_call___ngw__getFolderRequest(soap, endpoint, NULL, ngwm__getFolderRequest, ngwm__getFolderResponse) : SOAP_EOM; };
	virtual int __ngw__getFolderListRequest(_ngwm__getFolderListRequest *ngwm__getFolderListRequest, _ngwm__getFolderListResponse *ngwm__getFolderListResponse) { return soap ? soap_call___ngw__getFolderListRequest(soap, endpoint, NULL, ngwm__getFolderListRequest, ngwm__getFolderListResponse) : SOAP_EOM; };
	virtual int __ngw__getFreeBusyRequest(_ngwm__getFreeBusyRequest *ngwm__getFreeBusyRequest, _ngwm__getFreeBusyResponse *ngwm__getFreeBusyResponse) { return soap ? soap_call___ngw__getFreeBusyRequest(soap, endpoint, NULL, ngwm__getFreeBusyRequest, ngwm__getFreeBusyResponse) : SOAP_EOM; };
	virtual int __ngw__getItemRequest(_ngwm__getItemRequest *ngwm__getItemRequest, _ngwm__getItemResponse *ngwm__getItemResponse) { return soap ? soap_call___ngw__getItemRequest(soap, endpoint, NULL, ngwm__getItemRequest, ngwm__getItemResponse) : SOAP_EOM; };
	virtual int __ngw__getItemsRequest(_ngwm__getItemsRequest *ngwm__getItemsRequest, _ngwm__getItemsResponse *ngwm__getItemsResponse) { return soap ? soap_call___ngw__getItemsRequest(soap, endpoint, NULL, ngwm__getItemsRequest, ngwm__getItemsResponse) : SOAP_EOM; };
	virtual int __ngw__getJunkEntriesRequest(_ngwm__getJunkEntriesRequest *ngwm__getJunkEntriesRequest, _ngwm__getJunkEntriesResponse *ngwm__getJunkEntriesResponse) { return soap ? soap_call___ngw__getJunkEntriesRequest(soap, endpoint, NULL, ngwm__getJunkEntriesRequest, ngwm__getJunkEntriesResponse) : SOAP_EOM; };
	virtual int __ngw__getJunkMailSettingsRequest(_ngwm__getJunkMailSettingsRequest *ngwm__getJunkMailSettingsRequest, _ngwm__getJunkMailSettingsResponse *ngwm__getJunkMailSettingsResponse) { return soap ? soap_call___ngw__getJunkMailSettingsRequest(soap, endpoint, NULL, ngwm__getJunkMailSettingsRequest, ngwm__getJunkMailSettingsResponse) : SOAP_EOM; };
	virtual int __ngw__getLibraryItemRequest(_ngwm__getLibraryItemRequest *ngwm__getLibraryItemRequest, _ngwm__getLibraryItemResponse *ngwm__getLibraryItemResponse) { return soap ? soap_call___ngw__getLibraryItemRequest(soap, endpoint, NULL, ngwm__getLibraryItemRequest, ngwm__getLibraryItemResponse) : SOAP_EOM; };
	virtual int __ngw__getLibraryListRequest(_ngwm__getLibraryListRequest *ngwm__getLibraryListRequest, _ngwm__getLibraryListResponse *ngwm__getLibraryListResponse) { return soap ? soap_call___ngw__getLibraryListRequest(soap, endpoint, NULL, ngwm__getLibraryListRequest, ngwm__getLibraryListResponse) : SOAP_EOM; };
	virtual int __ngw__getQuickMessagesRequest(_ngwm__getQuickMessagesRequest *ngwm__getQuickMessagesRequest, _ngwm__getQuickMessagesResponse *ngwm__getQuickMessagesResponse) { return soap ? soap_call___ngw__getQuickMessagesRequest(soap, endpoint, NULL, ngwm__getQuickMessagesRequest, ngwm__getQuickMessagesResponse) : SOAP_EOM; };
	virtual int __ngw__getProxyAccessListRequest(_ngwm__getProxyAccessListRequest *ngwm__getProxyAccessListRequest, _ngwm__getProxyAccessListResponse *ngwm__getProxyAccessListResponse) { return soap ? soap_call___ngw__getProxyAccessListRequest(soap, endpoint, NULL, ngwm__getProxyAccessListRequest, ngwm__getProxyAccessListResponse) : SOAP_EOM; };
	virtual int __ngw__getProxyListRequest(_ngwm__getProxyListRequest *ngwm__getProxyListRequest, _ngwm__getProxyListResponse *ngwm__getProxyListResponse) { return soap ? soap_call___ngw__getProxyListRequest(soap, endpoint, NULL, ngwm__getProxyListRequest, ngwm__getProxyListResponse) : SOAP_EOM; };
	virtual int __ngw__getRuleListRequest(_ngwm__getRuleListRequest *ngwm__getRuleListRequest, _ngwm__getRuleListResponse *ngwm__getRuleListResponse) { return soap ? soap_call___ngw__getRuleListRequest(soap, endpoint, NULL, ngwm__getRuleListRequest, ngwm__getRuleListResponse) : SOAP_EOM; };
	virtual int __ngw__getSettingsRequest(_ngwm__getSettingsRequest *ngwm__getSettingsRequest, _ngwm__getSettingsResponse *ngwm__getSettingsResponse) { return soap ? soap_call___ngw__getSettingsRequest(soap, endpoint, NULL, ngwm__getSettingsRequest, ngwm__getSettingsResponse) : SOAP_EOM; };
	virtual int __ngw__getSignaturesRequest(_ngwm__getSignaturesRequest *ngwm__getSignaturesRequest, _ngwm__getSignaturesResponse *ngwm__getSignaturesResponse) { return soap ? soap_call___ngw__getSignaturesRequest(soap, endpoint, NULL, ngwm__getSignaturesRequest, ngwm__getSignaturesResponse) : SOAP_EOM; };
	virtual int __ngw__getTimestampRequest(_ngwm__getTimestampRequest *ngwm__getTimestampRequest, _ngwm__getTimestampResponse *ngwm__getTimestampResponse) { return soap ? soap_call___ngw__getTimestampRequest(soap, endpoint, NULL, ngwm__getTimestampRequest, ngwm__getTimestampResponse) : SOAP_EOM; };
	virtual int __ngw__getTimezoneListRequest(_ngwm__getTimezoneListRequest *ngwm__getTimezoneListRequest, _ngwm__getTimezoneListResponse *ngwm__getTimezoneListResponse) { return soap ? soap_call___ngw__getTimezoneListRequest(soap, endpoint, NULL, ngwm__getTimezoneListRequest, ngwm__getTimezoneListResponse) : SOAP_EOM; };
	virtual int __ngw__getUserListRequest(_ngwm__getUserListRequest *ngwm__getUserListRequest, _ngwm__getUserListResponse *ngwm__getUserListResponse) { return soap ? soap_call___ngw__getUserListRequest(soap, endpoint, NULL, ngwm__getUserListRequest, ngwm__getUserListResponse) : SOAP_EOM; };
	virtual int __ngw__loginRequest(_ngwm__loginRequest *ngwm__loginRequest, _ngwm__loginResponse *ngwm__loginResponse) { return soap ? soap_call___ngw__loginRequest(soap, endpoint, NULL, ngwm__loginRequest, ngwm__loginResponse) : SOAP_EOM; };
	virtual int __ngw__logoutRequest(_ngwm__logoutRequest *ngwm__logoutRequest, _ngwm__logoutResponse *ngwm__logoutResponse) { return soap ? soap_call___ngw__logoutRequest(soap, endpoint, NULL, ngwm__logoutRequest, ngwm__logoutResponse) : SOAP_EOM; };
	virtual int __ngw__markPrivateRequest(_ngwm__markPrivateRequest *ngwm__markPrivateRequest, _ngwm__markPrivateResponse *ngwm__markPrivateResponse) { return soap ? soap_call___ngw__markPrivateRequest(soap, endpoint, NULL, ngwm__markPrivateRequest, ngwm__markPrivateResponse) : SOAP_EOM; };
	virtual int __ngw__markReadRequest(_ngwm__markReadRequest *ngwm__markReadRequest, _ngwm__markReadResponse *ngwm__markReadResponse) { return soap ? soap_call___ngw__markReadRequest(soap, endpoint, NULL, ngwm__markReadRequest, ngwm__markReadResponse) : SOAP_EOM; };
	virtual int __ngw__markUnPrivateRequest(_ngwm__markUnPrivateRequest *ngwm__markUnPrivateRequest, _ngwm__markUnPrivateResponse *ngwm__markUnPrivateResponse) { return soap ? soap_call___ngw__markUnPrivateRequest(soap, endpoint, NULL, ngwm__markUnPrivateRequest, ngwm__markUnPrivateResponse) : SOAP_EOM; };
	virtual int __ngw__markUnReadRequest(_ngwm__markUnReadRequest *ngwm__markUnReadRequest, _ngwm__markUnReadResponse *ngwm__markUnReadResponse) { return soap ? soap_call___ngw__markUnReadRequest(soap, endpoint, NULL, ngwm__markUnReadRequest, ngwm__markUnReadResponse) : SOAP_EOM; };
	virtual int __ngw__modifyItemRequest(_ngwm__modifyItemRequest *ngwm__modifyItemRequest, _ngwm__modifyItemResponse *ngwm__modifyItemResponse) { return soap ? soap_call___ngw__modifyItemRequest(soap, endpoint, NULL, ngwm__modifyItemRequest, ngwm__modifyItemResponse) : SOAP_EOM; };
	virtual int __ngw__modifyJunkEntryRequest(_ngwm__modifyJunkEntryRequest *ngwm__modifyJunkEntryRequest, _ngwm__modifyJunkEntryResponse *ngwm__modifyJunkEntryResponse) { return soap ? soap_call___ngw__modifyJunkEntryRequest(soap, endpoint, NULL, ngwm__modifyJunkEntryRequest, ngwm__modifyJunkEntryResponse) : SOAP_EOM; };
	virtual int __ngw__modifyJunkMailSettingsRequest(_ngwm__modifyJunkMailSettingsRequest *ngwm__modifyJunkMailSettingsRequest, _ngwm__modifyJunkMailSettingsResponse *ngwm__modifyJunkMailSettingsResponse) { return soap ? soap_call___ngw__modifyJunkMailSettingsRequest(soap, endpoint, NULL, ngwm__modifyJunkMailSettingsRequest, ngwm__modifyJunkMailSettingsResponse) : SOAP_EOM; };
	virtual int __ngw__modifyPasswordRequest(_ngwm__modifyPasswordRequest *ngwm__modifyPasswordRequest, _ngwm__modifyPasswordResponse *ngwm__modifyPasswordResponse) { return soap ? soap_call___ngw__modifyPasswordRequest(soap, endpoint, NULL, ngwm__modifyPasswordRequest, ngwm__modifyPasswordResponse) : SOAP_EOM; };
	virtual int __ngw__modifyProxyAccessRequest(_ngwm__modifyProxyAccessRequest *ngwm__modifyProxyAccessRequest, _ngwm__modifyProxyAccessResponse *ngwm__modifyProxyAccessResponse) { return soap ? soap_call___ngw__modifyProxyAccessRequest(soap, endpoint, NULL, ngwm__modifyProxyAccessRequest, ngwm__modifyProxyAccessResponse) : SOAP_EOM; };
	virtual int __ngw__modifySettingsRequest(_ngwm__modifySettingsRequest *ngwm__modifySettingsRequest, _ngwm__modifySettingsResponse *ngwm__modifySettingsResponse) { return soap ? soap_call___ngw__modifySettingsRequest(soap, endpoint, NULL, ngwm__modifySettingsRequest, ngwm__modifySettingsResponse) : SOAP_EOM; };
	virtual int __ngw__modifySignaturesRequest(_ngwm__modifySignaturesRequest *ngwm__modifySignaturesRequest, _ngwm__modifySignaturesResponse *ngwm__modifySignaturesResponse) { return soap ? soap_call___ngw__modifySignaturesRequest(soap, endpoint, NULL, ngwm__modifySignaturesRequest, ngwm__modifySignaturesResponse) : SOAP_EOM; };
	virtual int __ngw__moveItemRequest(_ngwm__moveItemRequest *ngwm__moveItemRequest, _ngwm__moveItemResponse *ngwm__moveItemResponse) { return soap ? soap_call___ngw__moveItemRequest(soap, endpoint, NULL, ngwm__moveItemRequest, ngwm__moveItemResponse) : SOAP_EOM; };
	virtual int __ngw__positionCursorRequest(_ngwm__positionCursorRequest *ngwm__positionCursorRequest, _ngwm__positionCursorResponse *ngwm__positionCursorResponse) { return soap ? soap_call___ngw__positionCursorRequest(soap, endpoint, NULL, ngwm__positionCursorRequest, ngwm__positionCursorResponse) : SOAP_EOM; };
	virtual int __ngw__purgeDeletedItemsRequest(_ngwm__purgeDeletedItemsRequest *ngwm__purgeDeletedItemsRequest, _ngwm__purgeDeletedItemsResponse *ngwm__purgeDeletedItemsResponse) { return soap ? soap_call___ngw__purgeDeletedItemsRequest(soap, endpoint, NULL, ngwm__purgeDeletedItemsRequest, ngwm__purgeDeletedItemsResponse) : SOAP_EOM; };
	virtual int __ngw__purgeRequest(_ngwm__purgeRequest *ngwm__purgeRequest, _ngwm__purgeResponse *ngwm__purgeResponse) { return soap ? soap_call___ngw__purgeRequest(soap, endpoint, NULL, ngwm__purgeRequest, ngwm__purgeResponse) : SOAP_EOM; };
	virtual int __ngw__readCursorRequest(_ngwm__readCursorRequest *ngwm__readCursorRequest, _ngwm__readCursorResponse *ngwm__readCursorResponse) { return soap ? soap_call___ngw__readCursorRequest(soap, endpoint, NULL, ngwm__readCursorRequest, ngwm__readCursorResponse) : SOAP_EOM; };
	virtual int __ngw__removeCustomDefinitionRequest(_ngwm__removeCustomDefinitionRequest *ngwm__removeCustomDefinitionRequest, _ngwm__removeCustomDefinitionResponse *ngwm__removeCustomDefinitionResponse) { return soap ? soap_call___ngw__removeCustomDefinitionRequest(soap, endpoint, NULL, ngwm__removeCustomDefinitionRequest, ngwm__removeCustomDefinitionResponse) : SOAP_EOM; };
	virtual int __ngw__removeItemRequest(_ngwm__removeItemRequest *ngwm__removeItemRequest, _ngwm__removeItemResponse *ngwm__removeItemResponse) { return soap ? soap_call___ngw__removeItemRequest(soap, endpoint, NULL, ngwm__removeItemRequest, ngwm__removeItemResponse) : SOAP_EOM; };
	virtual int __ngw__removeItemsRequest(_ngwm__removeItemsRequest *ngwm__removeItemsRequest, _ngwm__removeItemsResponse *ngwm__removeItemsResponse) { return soap ? soap_call___ngw__removeItemsRequest(soap, endpoint, NULL, ngwm__removeItemsRequest, ngwm__removeItemsResponse) : SOAP_EOM; };
	virtual int __ngw__removeJunkEntryRequest(_ngwm__removeJunkEntryRequest *ngwm__removeJunkEntryRequest, _ngwm__removeJunkEntryResponse *ngwm__removeJunkEntryResponse) { return soap ? soap_call___ngw__removeJunkEntryRequest(soap, endpoint, NULL, ngwm__removeJunkEntryRequest, ngwm__removeJunkEntryResponse) : SOAP_EOM; };
	virtual int __ngw__removeMembersRequest(_ngwm__removeMembersRequest *ngwm__removeMembersRequest, _ngwm__removeMembersResponse *ngwm__removeMembersResponse) { return soap ? soap_call___ngw__removeMembersRequest(soap, endpoint, NULL, ngwm__removeMembersRequest, ngwm__removeMembersResponse) : SOAP_EOM; };
	virtual int __ngw__removeProxyAccessRequest(_ngwm__removeProxyAccessRequest *ngwm__removeProxyAccessRequest, _ngwm__removeProxyAccessResponse *ngwm__removeProxyAccessResponse) { return soap ? soap_call___ngw__removeProxyAccessRequest(soap, endpoint, NULL, ngwm__removeProxyAccessRequest, ngwm__removeProxyAccessResponse) : SOAP_EOM; };
	virtual int __ngw__removeProxyUserRequest(_ngwm__removeProxyUserRequest *ngwm__removeProxyUserRequest, _ngwm__removeProxyUserResponse *ngwm__removeProxyUserResponse) { return soap ? soap_call___ngw__removeProxyUserRequest(soap, endpoint, NULL, ngwm__removeProxyUserRequest, ngwm__removeProxyUserResponse) : SOAP_EOM; };
	virtual int __ngw__removeSignatureRequest(_ngwm__removeSignatureRequest *ngwm__removeSignatureRequest, _ngwm__removeSignatureResponse *ngwm__removeSignatureResponse) { return soap ? soap_call___ngw__removeSignatureRequest(soap, endpoint, NULL, ngwm__removeSignatureRequest, ngwm__removeSignatureResponse) : SOAP_EOM; };
	virtual int __ngw__replyRequest(_ngwm__replyRequest *ngwm__replyRequest, _ngwm__replyResponse *ngwm__replyResponse) { return soap ? soap_call___ngw__replyRequest(soap, endpoint, NULL, ngwm__replyRequest, ngwm__replyResponse) : SOAP_EOM; };
	virtual int __ngw__retractRequest(_ngwm__retractRequest *ngwm__retractRequest, _ngwm__retractResponse *ngwm__retractResponse) { return soap ? soap_call___ngw__retractRequest(soap, endpoint, NULL, ngwm__retractRequest, ngwm__retractResponse) : SOAP_EOM; };
	virtual int __ngw__sendItemRequest(_ngwm__sendItemRequest *ngwm__sendItemRequest, _ngwm__sendItemResponse *ngwm__sendItemResponse) { return soap ? soap_call___ngw__sendItemRequest(soap, endpoint, NULL, ngwm__sendItemRequest, ngwm__sendItemResponse) : SOAP_EOM; };
	virtual int __ngw__setTimestampRequest(_ngwm__setTimestampRequest *ngwm__setTimestampRequest, _ngwm__setTimestampResponse *ngwm__setTimestampResponse) { return soap ? soap_call___ngw__setTimestampRequest(soap, endpoint, NULL, ngwm__setTimestampRequest, ngwm__setTimestampResponse) : SOAP_EOM; };
	virtual int __ngw__startFreeBusySessionRequest(_ngwm__startFreeBusySessionRequest *ngwm__startFreeBusySessionRequest, _ngwm__startFreeBusySessionResponse *ngwm__startFreeBusySessionResponse) { return soap ? soap_call___ngw__startFreeBusySessionRequest(soap, endpoint, NULL, ngwm__startFreeBusySessionRequest, ngwm__startFreeBusySessionResponse) : SOAP_EOM; };
	virtual int __ngw__unacceptRequest(_ngwm__unacceptRequest *ngwm__unacceptRequest, _ngwm__unacceptResponse *ngwm__unacceptResponse) { return soap ? soap_call___ngw__unacceptRequest(soap, endpoint, NULL, ngwm__unacceptRequest, ngwm__unacceptResponse) : SOAP_EOM; };
	virtual int __ngw__uncompleteRequest(_ngwm__uncompleteRequest *ngwm__uncompleteRequest, _ngwm__uncompleteResponse *ngwm__uncompleteResponse) { return soap ? soap_call___ngw__uncompleteRequest(soap, endpoint, NULL, ngwm__uncompleteRequest, ngwm__uncompleteResponse) : SOAP_EOM; };
	virtual int __ngw__updateVersionStatusRequest(_ngwm__updateVersionStatusRequest *ngwm__updateVersionStatusRequest, _ngwm__updateVersionStatusResponse *ngwm__updateVersionStatusResponse) { return soap ? soap_call___ngw__updateVersionStatusRequest(soap, endpoint, NULL, ngwm__updateVersionStatusRequest, ngwm__updateVersionStatusResponse) : SOAP_EOM; };
	virtual int __ngw__cleanEventConfigurationRequest(_ngwe__cleanEventConfigurationRequest *ngwe__cleanEventConfigurationRequest, _ngwe__cleanEventConfigurationResponse *ngwe__cleanEventConfigurationResponse) { return soap ? soap_call___ngw__cleanEventConfigurationRequest(soap, endpoint, NULL, ngwe__cleanEventConfigurationRequest, ngwe__cleanEventConfigurationResponse) : SOAP_EOM; };
	virtual int __ngw__configureEventsRequest(_ngwe__configureEventsRequest *ngwe__configureEventsRequest, _ngwe__configureEventsResponse *ngwe__configureEventsResponse) { return soap ? soap_call___ngw__configureEventsRequest(soap, endpoint, NULL, ngwe__configureEventsRequest, ngwe__configureEventsResponse) : SOAP_EOM; };
	virtual int __ngw__getEventConfigurationRequest(_ngwe__getEventConfigurationRequest *ngwe__getEventConfigurationRequest, _ngwe__getEventConfigurationResponse *ngwe__getEventConfigurationResponse) { return soap ? soap_call___ngw__getEventConfigurationRequest(soap, endpoint, NULL, ngwe__getEventConfigurationRequest, ngwe__getEventConfigurationResponse) : SOAP_EOM; };
	virtual int __ngw__getEventsRequest(_ngwe__getEventsRequest *ngwe__getEventsRequest, _ngwe__getEventsResponse *ngwe__getEventsResponse) { return soap ? soap_call___ngw__getEventsRequest(soap, endpoint, NULL, ngwe__getEventsRequest, ngwe__getEventsResponse) : SOAP_EOM; };
	virtual int __ngw__removeEventConfigurationRequest(_ngwe__removeEventConfigurationRequest *ngwe__removeEventConfigurationRequest, _ngwe__removeEventConfigurationResponse *ngwe__removeEventConfigurationResponse) { return soap ? soap_call___ngw__removeEventConfigurationRequest(soap, endpoint, NULL, ngwe__removeEventConfigurationRequest, ngwe__removeEventConfigurationResponse) : SOAP_EOM; };
	virtual int __ngw__removeEventsRequest(_ngwe__removeEventsRequest *ngwe__removeEventsRequest, _ngwe__removeEventsResponse *ngwe__removeEventsResponse) { return soap ? soap_call___ngw__removeEventsRequest(soap, endpoint, NULL, ngwe__removeEventsRequest, ngwe__removeEventsResponse) : SOAP_EOM; };
};
#endif
