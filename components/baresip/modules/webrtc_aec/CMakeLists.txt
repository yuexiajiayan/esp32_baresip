project(webrtc_aec)




list(APPEND MODULES_DETECTED ${PROJECT_NAME})
set(MODULES_DETECTED ${MODULES_DETECTED} PARENT_SCOPE)

set(SRCS aec.cpp decode.cpp encode.cpp 
    webrtc/modules/audio_processing/aecm/aecm_core.cc                  
	webrtc/modules/audio_processing/aecm/echo_control_mobile.cc 
	webrtc/modules/audio_processing/aecm/aecm_core_neon.cc            
	webrtc/modules/audio_processing/aecm/aecm_core_c.cc        
	webrtc/modules/audio_processing/utility/delay_estimator_wrapper.cc 
	webrtc/modules/audio_processing/utility/delay_estimator.cc         
	webrtc/common_audio/third_party/ooura/fft_size_256/fft4g.cc       
	webrtc/common_audio/third_party/spl_sqrt_floor/spl_sqrt_floor.c   
	webrtc/common_audio/ring_buffer.c                                 
	webrtc/common_audio/signal_processing/complex_bit_reverse.c       
	webrtc/common_audio/signal_processing/complex_fft.c               
	webrtc/common_audio/signal_processing/copy_set_operations.c       
	webrtc/common_audio/signal_processing/cross_correlation.c         
	webrtc/common_audio/signal_processing/division_operations.c       
	webrtc/common_audio/signal_processing/downsample_fast.c           
	webrtc/common_audio/signal_processing/energy.c                    
	webrtc/common_audio/signal_processing/get_scaling_square.c        
	webrtc/common_audio/signal_processing/min_max_operations.c        
	webrtc/common_audio/signal_processing/randomization_functions.c   
	webrtc/common_audio/signal_processing/real_fft.c                  
	webrtc/common_audio/signal_processing/spl_init.c                  
	webrtc/common_audio/signal_processing/spl_sqrt.c                  
	webrtc/common_audio/signal_processing/vector_scaling_operations.c
              
	webrtc/common_audio/signal_processing/cross_correlation_neon.c    
	webrtc/common_audio/signal_processing/downsample_fast_neon.c      
	webrtc/common_audio/signal_processing/min_max_operations_neon.c
)

if(STATIC)
    add_library(${PROJECT_NAME} OBJECT ${SRCS})
else()
    add_library(${PROJECT_NAME} MODULE ${SRCS})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${WEBRTC_AEC_INCLUDE_DIRS} ./webrtc .)
target_link_libraries(${PROJECT_NAME} PRIVATE ${WEBRTC_AEC_LIBRARIES})
target_compile_options(${PROJECT_NAME} PRIVATE
  -isystem $(WEBRTC_AEC_INCLUDE_DIRS)
  -fPIC
  -Wno-shadow
  -Wno-pedantic
  -Wno-missing-field-initializers
  -Wno-unused-parameter
)
target_link_options(${PROJECT_NAME} PUBLIC -lstdc++)
