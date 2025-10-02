  
#include <re_types.h>
#include <re_mod.h>

extern const struct mod_export exports_g711;
extern const struct mod_export exports_auconv;
extern const struct mod_export exports_auresamp;
extern const struct mod_export exports_uuid; 
extern const struct mod_export exports_webrtc_aecm;

const struct mod_export *mod_table[] = {
 &exports_g711,
 &exports_uuid,
 &exports_auconv,
 &exports_auresamp,  
 &exports_webrtc_aecm,
 NULL
};
