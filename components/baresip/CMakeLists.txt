set(COMPONENT_ADD_INCLUDEDIRS
    include
    ../re/include
    modules/webrtc_aecm/
)

set(COMPONENT_SRCDIRS
    src
    modules/g711
    #modules/i2s
    modules/auconv
    modules/auresamp
    modules/uuid
    modules/webrtc_aecm 
 
)

list(APPEND COMPONENT_SRCS
     modules/webrtc_aec/webrtc/modules/audio_processing/aec/aec_core.c                    
	 modules/webrtc_aec/webrtc/modules/audio_processing/aec/aec_rdft.c                    
	 modules/webrtc_aec/webrtc/modules/audio_processing/aec/aec_resampler.c               
	 modules/webrtc_aec/webrtc/modules/audio_processing/aec/echo_cancellation.c           
	 modules/webrtc_aec/webrtc/modules/audio_processing/aecm/aecm_core.c                  
	 modules/webrtc_aec/webrtc/modules/audio_processing/aecm/echo_control_mobile.c        
	 modules/webrtc_aec/webrtc/modules/audio_processing/ns/noise_suppression.c            
	 modules/webrtc_aec/webrtc/modules/audio_processing/ns/noise_suppression_x.c          
	 modules/webrtc_aec/webrtc/modules/audio_processing/ns/ns_core.c                      
	 modules/webrtc_aec/webrtc/modules/audio_processing/ns/nsx_core.c                     
	 modules/webrtc_aec/webrtc/modules/audio_processing/utility/delay_estimator_wrapper.c 
	 modules/webrtc_aec/webrtc/modules/audio_processing/utility/delay_estimator.c         
	 modules/webrtc_aec/webrtc/common_audio/fft4g.c                                       
	 modules/webrtc_aec/webrtc/common_audio/ring_buffer.c                                 
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/complex_bit_reverse.c       
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/complex_fft.c               
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/copy_set_operations.c       
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/cross_correlation.c         
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/division_operations.c       
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/downsample_fast.c           
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/energy.c                    
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/get_scaling_square.c       
	 modules/webrtc_aecc/webrtc/common_audio/signal_processing/min_max_operations.c        
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/randomization_functions.c   
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/real_fft.c                  
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/spl_init.c                  
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/spl_sqrt.c                  
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/spl_sqrt_floor.c            
	 modules/webrtc_aec/webrtc/common_audio/signal_processing/vector_scaling_operations.c
) 

set(COMPONENT_ADD_LDFLAGS
    -Wl,--gc-sections
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
    -Wno-char-subscripts
    -DSHARE_PATH="\"/usr/share/baresip\""
    -Wno-error=address
    -DSTATIC=1 -DEXTCONFIG=1 -DNODNS
)

set(COMPONENT_CXXFLAGS
    -DSTATIC=1 -DEXTCONFIG=1 -DNODNS
)

idf_component_register(
    INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}
    SRC_DIRS ${COMPONENT_SRCDIRS}
    REQUIRES re
)