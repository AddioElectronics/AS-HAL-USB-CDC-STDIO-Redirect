#ifndef CHECK_MCU_CORE_H_
#define CHECK_MCU_CORE_H_


// For some reason __has_include("core_cm***.h") stopped working after installing a new version of Atmel Studio.
// Until I find a better way to check for core type, this will have to do for now.
// If anyone figures it out, please create a pull request.


#define  IS_MCU_CORE_CM0PLUS \
defined(__SAMD21E15A__) || defined(__ATSAMD21E15A__)	||	\
defined(__SAMD21E16A__) || defined(__ATSAMD21E16A__)	||	\
defined(__SAMD21E17A__) || defined(__ATSAMD21E17A__)	||	\
defined(__SAMD21E18A__) || defined(__ATSAMD21E18A__)	||	\
defined(__SAMD21G15A__) || defined(__ATSAMD21G15A__)	||	\
defined(__SAMD21G16A__) || defined(__ATSAMD21G16A__)	||	\
defined(__SAMD21G17A__) || defined(__ATSAMD21G17A__)	||	\
defined(__SAMD21G17AU__) || defined(__ATSAMD21G17AU__)	||	\
defined(__SAMD21G18A__) || defined(__ATSAMD21G18A__)	||	\
defined(__SAMD21G18AU__) || defined(__ATSAMD21G18AU__)	||	\
defined(__SAMD21J15A__) || defined(__ATSAMD21J15A__)	||	\
defined(__SAMD21J16A__) || defined(__ATSAMD21J16A__)	||	\
defined(__SAMD21J17A__) || defined(__ATSAMD21J17A__)	||	\
defined(__SAMD21J18A__) || defined(__ATSAMD21J18A__)	||	\
															\
defined(__SAMDA1E14A__) || defined(__ATSAMDA1E14A__)	||	\
defined(__SAMDA1E15A__) || defined(__ATSAMDA1E15A__)	||	\
defined(__SAMDA1E16A__) || defined(__ATSAMDA1E16A__)	||	\
defined(__SAMDA1G14A__) || defined(__ATSAMDA1G14A__)	||	\
defined(__SAMDA1G15A__) || defined(__ATSAMDA1G15A__)	||	\
defined(__SAMDA1G16A__) || defined(__ATSAMDA1G16A__)	||	\
defined(__SAMDA1J14A__) || defined(__ATSAMDA1J14A__)	||	\
defined(__SAMDA1J15A__) || defined(__ATSAMDA1J15A__)	||	\
defined(__SAMDA1J16A__) || defined(__ATSAMDA1J16A__)	||	\
															\
defined(__SAML21E18A__) || defined(__ATSAML21E18A__)	||	\
defined(__SAML21G18A__) || defined(__ATSAML21G18A__)	||	\
defined(__SAML21J18A__) || defined(__ATSAML21J18A__)	||	\
															\
defined(__SAML22G16A__) || defined(__ATSAML22G16A__)	||	\
defined(__SAML22G17A__) || defined(__ATSAML22G17A__)	||	\
defined(__SAML22G18A__) || defined(__ATSAML22G18A__)	||	\
defined(__SAML22J16A__) || defined(__ATSAML22J16A__)	||	\
defined(__SAML22J17A__) || defined(__ATSAML22J17A__)	||	\
defined(__SAML22J18A__) || defined(__ATSAML22J18A__)	||	\
defined(__SAML22N16A__) || defined(__ATSAML22N16A__)	||	\
defined(__SAML22N17A__) || defined(__ATSAML22N17A__)	||	\
defined(__SAML22N18A__) || defined(__ATSAML22N18A__)

#define IS_MCU_CORE_CM3 \
defined(__SAM3A4C__)	||	\
defined(__SAM3A8C__)	||	\
							\
defined(__SAM3X4C__)	||	\
defined(__SAM3X4E__)	||	\
defined(__SAM3X8C__)	||	\
defined(__SAM3X8E__)	||	\
defined(__SAM3X8H__)	||	\
							\
defined(__SAM3N00A__)	||	\
defined(__SAM3N0A__)	||	\
defined(__SAM3N00B__)	||	\
defined(__SAM3N0B__)	||	\
defined(__SAM3N0C__)	||	\
defined(__SAM3N1A__)	||	\
defined(__SAM3N1B__)	||	\
defined(__SAM3N1C__)	||	\
defined(__SAM3N2A__)	||	\
defined(__SAM3N2B__)	||	\
defined(__SAM3N2C__)	||	\
defined(__SAM3N4A__)	||	\
defined(__SAM3N4B__)	||	\
defined(__SAM3N4C__)	||	\
							\
defined(__SAM3S1A__)	||	\
defined(__SAM3S1B__)	||	\
defined(__SAM3S1C__)	||	\
defined(__SAM3S2A__)	||	\
defined(__SAM3S2B__)	||	\
defined(__SAM3S2C__)	||	\
defined(__SAM3S4A__)	||	\
defined(__SAM3S4B__)	||	\
defined(__SAM3S4C__)	||	\
							\
defined(__SAM3SD8B__)	||	\
defined(__SAM3SD8C__)	||	\
defined(__SAM3S8B__)	||	\
defined(__SAM3S8C__)	||	\
							\
defined(__SAM3U1C__)	||	\
defined(__SAM3U1E__)	||	\
defined(__SAM3U2C__)	||	\
defined(__SAM3U2E__)	||	\
defined(__SAM3U4C__)	||	\
defined(__SAM3U4E__)

#define IS_MCU_CORE_CM4 \
defined (__SAM4CP16B_0__) || defined (__SAM4CP16C_0__)	||	\
defined (__SAM4CP16B_1__) || defined (__SAM4CP16C_1__)	||	\
															\
defined (__SAM4E8C__)									||	\
defined (__SAM4E8E__)									||	\
defined (__SAM4E16C__)									||	\
defined (__SAM4E16E__)									||	\
															\
defined(__SAM4LC8A__) || defined(__ATSAM4LC8A__)		||	\
defined(__SAM4LC8B__) || defined(__ATSAM4LC8B__)		||	\
defined(__SAM4LC8C__) || defined(__ATSAM4LC8C__)		||	\
defined(__SAM4LS8A__) || defined(__ATSAM4LS8A__)		||	\
defined(__SAM4LS8B__) || defined(__ATSAM4LS8B__)		||	\
defined(__SAM4LS8C__) || defined(__ATSAM4LS8C__)

#define IS_MCU_CORE_CM4F \
defined(__SAMD51G18A__) || defined(__ATSAMD51G18A__)	||	\
defined(__SAMD51G19A__) || defined(__ATSAMD51G19A__)	||	\
defined(__SAMD51J18A__) || defined(__ATSAMD51J18A__)	||	\
defined(__SAMD51J19A__) || defined(__ATSAMD51J19A__)	||	\
defined(__SAMD51J19B__) || defined(__ATSAMD51J19B__)	||	\
defined(__SAMD51J20A__) || defined(__ATSAMD51J20A__)	||	\
defined(__SAMD51J20C__) || defined(__ATSAMD51J20C__)	||	\
defined(__SAMD51N19A__) || defined(__ATSAMD51N19A__)	||	\
defined(__SAMD51N20A__) || defined(__ATSAMD51N20A__)	||	\
defined(__SAMD51P19A__) || defined(__ATSAMD51P19A__)	||	\
defined(__SAMD51P20A__) || defined(__ATSAMD51P20A__)	||	\
															\
defined(__SAME51G18A__) || defined(__ATSAME51G18A__)	||	\
defined(__SAME51G19A__) || defined(__ATSAME51G19A__)	||	\
defined(__SAME51J18A__) || defined(__ATSAME51J18A__)	||	\
defined(__SAME51J19A__) || defined(__ATSAME51J19A__)	||	\
defined(__SAME51J20A__) || defined(__ATSAME51J20A__)	||	\
defined(__SAME51N19A__) || defined(__ATSAME51N19A__)	||	\
defined(__SAME51N20A__) || defined(__ATSAME51N20A__)	||	\
															\
defined(__SAME53J18A__) || defined(__ATSAME53J18A__)	||	\
defined(__SAME53J19A__) || defined(__ATSAME53J19A__)	||	\
defined(__SAME53J20A__) || defined(__ATSAME53J20A__)	||	\
defined(__SAME53N19A__) || defined(__ATSAME53N19A__)	||	\
defined(__SAME53N20A__) || defined(__ATSAME53N20A__)	||	\
															\
defined(__SAME54N19A__) || defined(__ATSAME54N19A__)	||	\
defined(__SAME54N20A__) || defined(__ATSAME54N20A__)	||	\
defined(__SAME54P19A__) || defined(__ATSAME54P19A__)	||	\
defined(__SAME54P20A__) || defined(__ATSAME54P20A__)

#define IS_MCU_CORE_CM7 \
defined(__SAME70Q21__)	||	\
defined(__SAME70Q20__)	||	\
defined(__SAME70Q19__)	||	\
defined(__SAME70N21__)	||	\
defined(__SAME70N20__)	||	\
defined(__SAME70N19__)	||	\
defined(__SAME70J21__)	||	\
defined(__SAME70J20__)	||	\
defined(__SAME70J19__)	||	\
							\
defined(__SAMS70Q21__)	||	\
defined(__SAMS70Q20__)	||	\
defined(__SAMS70Q19__)	||	\
defined(__SAMS70N21__)	||	\
defined(__SAMS70N20__)	||	\
defined(__SAMS70N19__)	||	\
defined(__SAMS70J21__)	||	\
defined(__SAMS70J20__)	||	\
defined(__SAMS70J19__)	||	\
							\
defined(__SAMV70Q21__)	||	\
defined(__SAMV70Q20__)	||	\
defined(__SAMV70Q19__)	||	\
defined(__SAMV70N21__)	||	\
defined(__SAMV70N20__)	||	\
defined(__SAMV70N19__)	||	\
defined(__SAMV70J21__)	||	\
defined(__SAMV70J20__)	||	\
defined(__SAMV70J19__)	||	\
							\
defined(__SAMV71Q21__)	||	\
defined(__SAMV71Q20__)	||	\
defined(__SAMV71Q19__)	||	\
defined(__SAMV71N21__)	||	\
defined(__SAMV71N20__)	||	\
defined(__SAMV71N19__)	||	\
defined(__SAMV71J21__)	||	\
defined(__SAMV71J20__)	||	\
defined(__SAMV71J19__)	


#endif /* CHECK_MCU_CORE_H_ */