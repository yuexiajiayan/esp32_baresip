set(COMPONENT_ADD_INCLUDEDIRS
    include
)

set(COMPONENT_SRCDIRS
rem/fir
    rem/au
    rem/goertzel
    rem/dtmf
    rem/auresamp
    rem/aac
    rem/aumix
    rem/g711
    rem/auconv
    rem/autone
    rem/vid
    rem/vidconv
    rem/aufile
    rem/aubuf
    rem/auframe
    rem/aulevel 
    src
    src/main
    src/sys
    src/dns
    src/mbuf
    src/list
    src/mqueue
    src/mem
    src/tcp
    src/msg
    src/sipevent
    src/udp
    src/tmr
    src/fmt
    src/bfcp
    src/json
     src/aes
    src/dbg
     src/md5
     src/sa
    src/sdp
     src/hash
    src/httpauth
     src/uri 
     src/stun
    src/srtp
    src/ice
    src/turn
     src/mod
     src/sip
     src/websock
    src/conf
    src/hmac
    src/rtp 
    src/sipreg
    src/base64
     src/sipsess
     src/odict 
     src/http
    src/net
     src/telev
    src/sha
    src/crc32
    src/thread
    src/async
    src/rtpext
    src/net/posix
    src/btrace 
)

set(COMPONENT_OBJEXCLUDE
    re/src/main/openssl.o
    re/src/main/epoll.o
    re/src/dns/res.o
    re/src/mod/dl.o
    re/src/lock/rwlock.o
    re/src/net/ifaddrs.o
)

set(COMPONENT_CFLAGS
    -DHAVE_SELECT -DHAVE_SELECT_H
    -DHAVE_INET_NTOP -DHAVE_INET_PTON -DHAVE_PTHREAD -DHAVE_STRERROR_R
    -DHAVE_INTTYPES_H -DHAVE_STDBOOL_H -DHAVE_FORK
    -DHAVE_PWD_H
    -DHAVE_SIGNAL -DHAVE_SYS_TIME_H
    -DHAVE_UNISTD_H -DHAVE_STRINGS_H
    -DHAVE_ROUTE_LIST
    -Wall
    -Wmissing-declarations
    -Wmissing-prototypes
    -Wbad-function-cast
    -Wnested-externs
    -Wshadow
    -Waggregate-return
    -DLINUX -g -Os
    -Wno-error=char-subscripts
    -Wno-error=implicit-function-declaration
)

 
idf_component_register(
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    SRC_DIRS ${COMPONENT_SRCDIRS} 
    REQUIRES mbedtls esp_netif pthread esp_timer
)