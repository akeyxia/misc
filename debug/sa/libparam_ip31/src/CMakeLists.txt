add_library(param_ip31_objs OBJECT libparam_ip31.c)
add_executable(ip31-boot-reason-test ip31-write-param.c 
				libparam_ip31.c 
              )

install(TARGETS ip31-boot-reason-test RUNTIME DESTINATION usr/bin)
