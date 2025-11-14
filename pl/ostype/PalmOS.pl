# $Id$

$math              = "";
$thread            = "";
$shared = "";
$library_suffix    = ".a";
$executable_suffix = "";
$lib_type = "static";

use_backend("palm");
param_set("freetype2",            0);
param_set("stb-image",            1);

if($sys_lib_dir eq "" or $sys_include_dir eq "") {
    print("ERROR: PalmOS target requires sys_lib_dir and sys_include_dir to be set. \n");
    return 0;
}

$incdir = "${incdir} -I${sys_include_dir}/Core";
$incdir = "${incdir} -I${sys_include_dir}/Core/Hardware";
$incdir = "${incdir} -I${sys_include_dir}/Core/System";
$incdir = "${incdir} -I${sys_include_dir}/Core/UI";
$incdir = "${incdir} -I${sys_include_dir}/Dynamic";
$incdir = "${incdir} -I${sys_include_dir}/Libraries";

# The PalmOS SDK loves modern GCC
add_cflags("-Wno-attributes -Wno-unknown-pragmas -Wno-redefine");

add_cflags("-DSTBI_NO_THREAD_LOCALS -ffunction-sections -fdata-sections");
# "most of those are absolutely mandatory to build PalmOS code correctly" - the guy who made this retro68 fork
add_cflags("-Wno-multichar -funsafe-math-optimizations -Os -m68000 -mno-align-int -mpcrel -fpic -fshort-enums -mshort");

add_ldflags("-Wl,--gc-sections -Wl,-T ${cwd}/src/backend/palm.lkr");

1;
