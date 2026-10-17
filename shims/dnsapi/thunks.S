    .text
    .globl DnsAcquireContextHandle_A
DnsAcquireContextHandle_A:
    jmp *real+0(%rip)
    .globl DnsAcquireContextHandle_UTF8
DnsAcquireContextHandle_UTF8:
    jmp *real+8(%rip)
    .globl DnsAcquireContextHandle_W
DnsAcquireContextHandle_W:
    jmp *real+16(%rip)
    .globl DnsAddRecordSet_A
DnsAddRecordSet_A:
    jmp *real+24(%rip)
    .globl DnsAddRecordSet_UTF8
DnsAddRecordSet_UTF8:
    jmp *real+32(%rip)
    .globl DnsAddRecordSet_W
DnsAddRecordSet_W:
    jmp *real+40(%rip)
    .globl DnsAllocateRecord
DnsAllocateRecord:
    jmp *real+48(%rip)
    .globl DnsApiHeapReset
DnsApiHeapReset:
    jmp *real+56(%rip)
    .globl DnsAsyncRegisterHostAddrs_A
DnsAsyncRegisterHostAddrs_A:
    jmp *real+64(%rip)
    .globl DnsAsyncRegisterHostAddrs_UTF8
DnsAsyncRegisterHostAddrs_UTF8:
    jmp *real+72(%rip)
    .globl DnsAsyncRegisterHostAddrs_W
DnsAsyncRegisterHostAddrs_W:
    jmp *real+80(%rip)
    .globl DnsAsyncRegisterInit
DnsAsyncRegisterInit:
    jmp *real+88(%rip)
    .globl DnsAsyncRegisterTerm
DnsAsyncRegisterTerm:
    jmp *real+96(%rip)
    .globl DnsCheckNameCollision_A
DnsCheckNameCollision_A:
    jmp *real+104(%rip)
    .globl DnsCheckNameCollision_UTF8
DnsCheckNameCollision_UTF8:
    jmp *real+112(%rip)
    .globl DnsCheckNameCollision_W
DnsCheckNameCollision_W:
    jmp *real+120(%rip)
    .globl DnsCopyStringEx
DnsCopyStringEx:
    jmp *real+128(%rip)
    .globl DnsCreateReverseNameStringForIpAddress
DnsCreateReverseNameStringForIpAddress:
    jmp *real+136(%rip)
    .globl DnsCreateStandardDnsNameCopy
DnsCreateStandardDnsNameCopy:
    jmp *real+144(%rip)
    .globl DnsCreateStringCopy
DnsCreateStringCopy:
    jmp *real+152(%rip)
    .globl DnsDhcpSrvRegisterHostName_W
DnsDhcpSrvRegisterHostName_W:
    jmp *real+160(%rip)
    .globl DnsDhcpSrvRegisterInit
DnsDhcpSrvRegisterInit:
    jmp *real+168(%rip)
    .globl DnsDhcpSrvRegisterTerm
DnsDhcpSrvRegisterTerm:
    jmp *real+176(%rip)
    .globl DnsDisableAdapterDomainNameRegistration
DnsDisableAdapterDomainNameRegistration:
    jmp *real+184(%rip)
    .globl DnsDisableBNodeResolverThread
DnsDisableBNodeResolverThread:
    jmp *real+192(%rip)
    .globl DnsDisableDynamicRegistration
DnsDisableDynamicRegistration:
    jmp *real+200(%rip)
    .globl DnsDowncaseDnsNameLabel
DnsDowncaseDnsNameLabel:
    jmp *real+208(%rip)
    .globl DnsEnableAdapterDomainNameRegistration
DnsEnableAdapterDomainNameRegistration:
    jmp *real+216(%rip)
    .globl DnsEnableBNodeResolverThread
DnsEnableBNodeResolverThread:
    jmp *real+224(%rip)
    .globl DnsEnableDynamicRegistration
DnsEnableDynamicRegistration:
    jmp *real+232(%rip)
    .globl DnsExtractRecordsFromMessage_UTF8
DnsExtractRecordsFromMessage_UTF8:
    jmp *real+240(%rip)
    .globl DnsExtractRecordsFromMessage_W
DnsExtractRecordsFromMessage_W:
    jmp *real+248(%rip)
    .globl DnsFindAuthoritativeZone
DnsFindAuthoritativeZone:
    jmp *real+256(%rip)
    .globl DnsFlushResolverCache
DnsFlushResolverCache:
    jmp *real+264(%rip)
    .globl DnsFlushResolverCacheEntry_A
DnsFlushResolverCacheEntry_A:
    jmp *real+272(%rip)
    .globl DnsFlushResolverCacheEntry_UTF8
DnsFlushResolverCacheEntry_UTF8:
    jmp *real+280(%rip)
    .globl DnsFlushResolverCacheEntry_W
DnsFlushResolverCacheEntry_W:
    jmp *real+288(%rip)
    .globl DnsFree
DnsFree:
    jmp *real+296(%rip)
    .globl DnsFreeAdapterInformation
DnsFreeAdapterInformation:
    jmp *real+304(%rip)
    .globl DnsFreeNetworkInformation
DnsFreeNetworkInformation:
    jmp *real+312(%rip)
    .globl DnsFreeSearchInformation
DnsFreeSearchInformation:
    jmp *real+320(%rip)
    .globl DnsGetBufferLengthForStringCopy
DnsGetBufferLengthForStringCopy:
    jmp *real+328(%rip)
    .globl DnsGetCacheDataTable
DnsGetCacheDataTable:
    jmp *real+336(%rip)
    .globl DnsGetDnsServerList
DnsGetDnsServerList:
    jmp *real+344(%rip)
    .globl DnsGetDomainName
DnsGetDomainName:
    jmp *real+352(%rip)
    .globl DnsGetHostName_A
DnsGetHostName_A:
    jmp *real+360(%rip)
    .globl DnsGetHostName_UTF8
DnsGetHostName_UTF8:
    jmp *real+368(%rip)
    .globl DnsGetHostName_W
DnsGetHostName_W:
    jmp *real+376(%rip)
    .globl DnsGetIpAddressInfoList
DnsGetIpAddressInfoList:
    jmp *real+384(%rip)
    .globl DnsGetIpAddressList
DnsGetIpAddressList:
    jmp *real+392(%rip)
    .globl DnsGetLastServerUpdateIP
DnsGetLastServerUpdateIP:
    jmp *real+400(%rip)
    .globl DnsGetMaxNumberOfAddressesToRegister
DnsGetMaxNumberOfAddressesToRegister:
    jmp *real+408(%rip)
    .globl DnsGetNetworkInformation
DnsGetNetworkInformation:
    jmp *real+416(%rip)
    .globl DnsGetPrimaryDomainName_A
DnsGetPrimaryDomainName_A:
    jmp *real+424(%rip)
    .globl DnsGetPrimaryDomainName_UTF8
DnsGetPrimaryDomainName_UTF8:
    jmp *real+432(%rip)
    .globl DnsGetPrimaryDomainName_W
DnsGetPrimaryDomainName_W:
    jmp *real+440(%rip)
    .globl DnsGetSearchInformation
DnsGetSearchInformation:
    jmp *real+448(%rip)
    .globl DnsIpv6AddressToString
DnsIpv6AddressToString:
    jmp *real+456(%rip)
    .globl DnsIpv6StringToAddress
DnsIpv6StringToAddress:
    jmp *real+464(%rip)
    .globl DnsIsAMailboxType
DnsIsAMailboxType:
    jmp *real+472(%rip)
    .globl DnsIsAdapterDomainNameRegistrationEnabled
DnsIsAdapterDomainNameRegistrationEnabled:
    jmp *real+480(%rip)
    .globl DnsIsDynamicRegistrationEnabled
DnsIsDynamicRegistrationEnabled:
    jmp *real+488(%rip)
    .globl DnsIsStatusRcode
DnsIsStatusRcode:
    jmp *real+496(%rip)
    .globl DnsIsStringCountValidForTextType
DnsIsStringCountValidForTextType:
    jmp *real+504(%rip)
    .globl DnsMapRcodeToStatus
DnsMapRcodeToStatus:
    jmp *real+512(%rip)
    .globl DnsModifyRecordSet_A
DnsModifyRecordSet_A:
    jmp *real+520(%rip)
    .globl DnsModifyRecordSet_UTF8
DnsModifyRecordSet_UTF8:
    jmp *real+528(%rip)
    .globl DnsModifyRecordSet_W
DnsModifyRecordSet_W:
    jmp *real+536(%rip)
    .globl DnsModifyRecordsInSet_A
DnsModifyRecordsInSet_A:
    jmp *real+544(%rip)
    .globl DnsModifyRecordsInSet_UTF8
DnsModifyRecordsInSet_UTF8:
    jmp *real+552(%rip)
    .globl DnsModifyRecordsInSet_W
DnsModifyRecordsInSet_W:
    jmp *real+560(%rip)
    .globl DnsNameCompareEx_A
DnsNameCompareEx_A:
    jmp *real+568(%rip)
    .globl DnsNameCompareEx_UTF8
DnsNameCompareEx_UTF8:
    jmp *real+576(%rip)
    .globl DnsNameCompareEx_W
DnsNameCompareEx_W:
    jmp *real+584(%rip)
    .globl DnsNameCompare_A
DnsNameCompare_A:
    jmp *real+592(%rip)
    .globl DnsNameCompare_W
DnsNameCompare_W:
    jmp *real+600(%rip)
    .globl DnsNameCopy
DnsNameCopy:
    jmp *real+608(%rip)
    .globl DnsNameCopyAllocate
DnsNameCopyAllocate:
    jmp *real+616(%rip)
    .globl DnsNotifyResolver
DnsNotifyResolver:
    jmp *real+624(%rip)
    .globl DnsQueryConfig
DnsQueryConfig:
    jmp *real+632(%rip)
    .globl DnsQueryEx
DnsQueryEx:
    jmp *real+640(%rip)
    .globl DnsQuery_A
DnsQuery_A:
    jmp *real+648(%rip)
    .globl DnsQuery_UTF8
DnsQuery_UTF8:
    jmp *real+656(%rip)
    .globl DnsQuery_W
DnsQuery_W:
    jmp *real+664(%rip)
    .globl DnsRecordBuild_UTF8
DnsRecordBuild_UTF8:
    jmp *real+672(%rip)
    .globl DnsRecordBuild_W
DnsRecordBuild_W:
    jmp *real+680(%rip)
    .globl DnsRecordCompare
DnsRecordCompare:
    jmp *real+688(%rip)
    .globl DnsRecordCopyEx
DnsRecordCopyEx:
    jmp *real+696(%rip)
    .globl DnsRecordListFree
DnsRecordListFree:
    jmp *real+704(%rip)
    .globl DnsRecordSetCompare
DnsRecordSetCompare:
    jmp *real+712(%rip)
    .globl DnsRecordSetCopyEx
DnsRecordSetCopyEx:
    jmp *real+720(%rip)
    .globl DnsRecordSetDetach
DnsRecordSetDetach:
    jmp *real+728(%rip)
    .globl DnsRecordStringForType
DnsRecordStringForType:
    jmp *real+736(%rip)
    .globl DnsRecordStringForWritableType
DnsRecordStringForWritableType:
    jmp *real+744(%rip)
    .globl DnsRecordTypeForName
DnsRecordTypeForName:
    jmp *real+752(%rip)
    .globl DnsRelationalCompare_UTF8
DnsRelationalCompare_UTF8:
    jmp *real+760(%rip)
    .globl DnsRelationalCompare_W
DnsRelationalCompare_W:
    jmp *real+768(%rip)
    .globl DnsReleaseContextHandle
DnsReleaseContextHandle:
    jmp *real+776(%rip)
    .globl DnsRemoveRegistrations
DnsRemoveRegistrations:
    jmp *real+784(%rip)
    .globl DnsReplaceRecordSetA
DnsReplaceRecordSetA:
    jmp *real+792(%rip)
    .globl DnsReplaceRecordSetUTF8
DnsReplaceRecordSetUTF8:
    jmp *real+800(%rip)
    .globl DnsReplaceRecordSetW
DnsReplaceRecordSetW:
    jmp *real+808(%rip)
    .globl DnsReplaceRecordSet_A
DnsReplaceRecordSet_A:
    jmp *real+816(%rip)
    .globl DnsReplaceRecordSet_UTF8
DnsReplaceRecordSet_UTF8:
    jmp *real+824(%rip)
    .globl DnsReplaceRecordSet_W
DnsReplaceRecordSet_W:
    jmp *real+832(%rip)
    .globl DnsServiceBrowse
DnsServiceBrowse:
    jmp *real+840(%rip)
    .globl DnsServiceNotificationDeregister_A
DnsServiceNotificationDeregister_A:
    jmp *real+848(%rip)
    .globl DnsServiceNotificationDeregister_UTF8
DnsServiceNotificationDeregister_UTF8:
    jmp *real+856(%rip)
    .globl DnsServiceNotificationDeregister_W
DnsServiceNotificationDeregister_W:
    jmp *real+864(%rip)
    .globl DnsServiceNotificationRegister_A
DnsServiceNotificationRegister_A:
    jmp *real+872(%rip)
    .globl DnsServiceNotificationRegister_UTF8
DnsServiceNotificationRegister_UTF8:
    jmp *real+880(%rip)
    .globl DnsServiceNotificationRegister_W
DnsServiceNotificationRegister_W:
    jmp *real+888(%rip)
    .globl DnsSetMaxNumberOfAddressesToRegister
DnsSetMaxNumberOfAddressesToRegister:
    jmp *real+896(%rip)
    .globl DnsStatusString
DnsStatusString:
    jmp *real+904(%rip)
    .globl DnsStringCopyAllocateEx
DnsStringCopyAllocateEx:
    jmp *real+912(%rip)
    .globl DnsUnicodeToUtf8
DnsUnicodeToUtf8:
    jmp *real+920(%rip)
    .globl DnsUpdate
DnsUpdate:
    jmp *real+928(%rip)
    .globl DnsUpdateTest_A
DnsUpdateTest_A:
    jmp *real+936(%rip)
    .globl DnsUpdateTest_UTF8
DnsUpdateTest_UTF8:
    jmp *real+944(%rip)
    .globl DnsUpdateTest_W
DnsUpdateTest_W:
    jmp *real+952(%rip)
    .globl DnsUtf8ToUnicode
DnsUtf8ToUnicode:
    jmp *real+960(%rip)
    .globl DnsValidateName_A
DnsValidateName_A:
    jmp *real+968(%rip)
    .globl DnsValidateName_UTF8
DnsValidateName_UTF8:
    jmp *real+976(%rip)
    .globl DnsValidateName_W
DnsValidateName_W:
    jmp *real+984(%rip)
    .globl DnsValidateUtf8Byte
DnsValidateUtf8Byte:
    jmp *real+992(%rip)
    .globl DnsWinsRecordFlagForString
DnsWinsRecordFlagForString:
    jmp *real+1000(%rip)
    .globl DnsWinsRecordFlagString
DnsWinsRecordFlagString:
    jmp *real+1008(%rip)
    .globl DnsWriteQuestionToBuffer_UTF8
DnsWriteQuestionToBuffer_UTF8:
    jmp *real+1016(%rip)
    .globl DnsWriteQuestionToBuffer_W
DnsWriteQuestionToBuffer_W:
    jmp *real+1024(%rip)
    .globl DnsWriteReverseNameStringForIpAddress
DnsWriteReverseNameStringForIpAddress:
    jmp *real+1032(%rip)
    .globl GetCurrentTimeInSeconds
GetCurrentTimeInSeconds:
    jmp *real+1040(%rip)
