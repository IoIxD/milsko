#!/usr/bin/env perl
# $Id$
our $target = `uname -s`;
$target =~ s/\r?\n$//;

foreach my $l (@ARGV) {
    if ($l =~ /^--.+$/) {
    }
    elsif ($l =~ /^(.+)=(.+)$/) {
        $ENV{$1} = $2;
    }
}

# stackoverflow tells me this is how we get the pwd
use Cwd qw();
our $cwd = Cwd::cwd();


our $cc      = defined($ENV{CC}) ? $ENV{CC} : "gcc";
our $ar      = defined($ENV{AR}) ? $ENV{AR} : "ar";
our $incdir  = "-I include";
our $cflags  = "-fPIC -D_MILSKO";
our $libdir  = "";
our $ldflags = "";
our $math    = "-lm";
our $thread  = "-lpthread";
our $shared  = "-shared";
our $backend = "";
our $lib_type = "shared";

our $library_prefix    = "lib";
our $library_suffix    = ".so";
our $object_suffix     = ".o";
our $executable_suffix = "";

our @library_targets  = ();
our @examples_targets = ();
our %examples_libs    = ();

our $sys_include_dir = "";
our $sys_lib_dir = "";

our $example_mw_link = "-lMw";

our $cross = 0;

require("./pl/utils.pl");



param_set("classic-theme",        0);
param_set("stb-image",            1);
param_set("stb-truetype",         0);
param_set("freetype2",            1);
param_set("xrender",              1);
param_set("opengl",               0);
param_set("vulkan",               0);
param_set("vulkan-string-helper", 1);

my %features = (
    "classic-theme"        => "use classic theme",
    "stb-image"            => "use stb_image instead of libjpeg/libpng",
    "stb-truetype"         => "use stb_truetype",
    "freetype2"            => "use FreeType2",
    "xrender"              => "use XRender",
    "opengl"               => "build OpenGL widget",
    "vulkan"               => "build Vulkan widget",
    "vulkan-string-helper" => "use Vulkan string helper"
);
my @features_keys = (
    "1classic-theme", "1stb-image",
    "1stb-truetype",  "1freetype2",
    "1opengl",        "2xrender",
    "1vulkan",        "2vulkan-string-helper"
);

foreach my $l (@ARGV) {
    if ($l =~ /^--(with|enable)-(.+)$/) {
        param_set($2, 1);
    }
    elsif ($l =~ /^--(without|disable)-(.+)$/) {
        param_set($2, 0);
    }
    elsif ($l =~ /^--target=(.+)$/) {
        $target = $1;
    }
    elsif ($l eq "--cross") {
        $cross = 1;
    }
    elsif ($l =~ /^--lib_type=(.+)$/) {
        $lib_type = $1;
    }
    # for systems where an SDK is needed (i.e. PalmOS, classic Mac)
    elsif ($l =~ /^--sys_include_dir=(.+)$/) {
        $sys_include_dir = $1;
        $incdir = "${incdir} -I${sys_include_dir}"
    } elsif ($l =~ /^--sys_lib_dir=(.+)$/) {
        $sys_lib_dir = $1;
        $cflags = "${cflags} -L${sys_lib_dir}"
    }
    elsif (($l eq "-h") or ($l eq "--help")) {
        print("Milsko Toolkit Configuration Utility\n");
        print("\n");
        print("Usage: $0 [options]\n");
        print("\n");
        print("Options:\n");
        print("  -h --help                       Display this help\n");
        print("  --target=TARGET                 Specify target\n");
        print("  --cross                         Indicate cross compilation\n");
        print("\n");
        print("Features:\n");
        print("  --enable-FEATURE                Use FEATURE\n");
        print("  --with-FEATURE                  Use FEATURE\n");
        print("  --disable-FEATURE               Do not use FEATURE\n");
        print("  --without-FEATURE               Do not use FEATURE\n");

        foreach my $l (@features_keys) {
            my $flag = (
                  (substr($l, 0, 1) eq '1')
                ? (param_get(substr($l, 1)) ? "--disable-" : "--enable-")
                : (param_get(substr($l, 1)) ? "--without-" : "--with-")
            ) . substr($l, 1);
            my $do   = param_get(substr($l, 1)) ? "Do not " : "";
            my $feat = $features{ substr($l, 1) };
            if (not(param_get($l))) {
                $feat = uc(substr($feat, 0, 1)) . substr($feat, 1);
            }
            print("  $flag" . (" " x (32 - length($flag))) . "${do}${feat}\n");
        }
        exit(0);
    }
}

if (-f "./pl/ostype/${target}.pl") {
    require("./pl/ostype/${target}.pl");
}
else {
    print(
"Perl file (pl/ostype/${target}.pl) was not found for your target. Please add one.\n"
    );
    exit(1);
}

require("./pl/rules.pl");

if($lib_type eq "static") {
    $example_mw_link = "-L${cwd}/src/ -static -lMw";
    $library_suffix = ".a";
};

print("Target : " . $target . "\n");

my @l = ();
foreach my $e (param_list()) {
    if (not(param_get($e))) {
        next;
    }
    if (($e eq "vulkan-string-helper") and param_get("vulkan")) {
        push(@l, $e);
    }
    elsif (($e eq "xrender") and ($backend eq "x11")) {
        push(@l, $e);
    }
    elsif (not($e eq "vulkan-string-helper") and not($e eq "xrender")) {
        push(@l, $e);
    }
}
print("Enabled: " . join(" ", @l) . "\n");

open(OUT, ">", "Makefile");
print(OUT "AR = ${ar}\n");
print(OUT "CC = ${cc}\n");
print(OUT "INCDIR = ${incdir}\n");
print(OUT "CFLAGS = ${cflags}\n");
print(OUT "LIBDIR = ${libdir}\n");
print(OUT "LDFLAGS = ${ldflags}\n");
print(OUT "LIBS = ${math} ${thread} ${libs}\n");
print(OUT "MATH = ${math}\n");
print(OUT "SHARED = ${shared}\n");
print(OUT "\n");
print(OUT ".PHONY: all format clean distclean lib examples\n");
print(OUT "\n");
print(OUT "all: lib examples\n");
print(OUT "\n");
print(OUT "format:\n");
print(OUT
"	clang-format --verbose -i `find src include -name \"*.c\" -or -name \"*.h\"`\n"
);
print(OUT
"	perltidy -b -bext=\"/\" --paren-tightness=2 `find tools pl Makefile.pl -name \"*.pl\"`\n"
);
print(OUT "\n");
print(OUT "lib: src/${library_prefix}Mw${library_suffix}\n");
print(OUT "\n");
print(  OUT "src/${library_prefix}Mw${library_suffix}: "
      . join(" ", @library_targets)
      . "\n");

if($lib_type ne "static") {
print(OUT
"	\$(CC) \$(SHARED) \$(LDFLAGS\) \$(LIBDIR) -o src/${library_prefix}Mw${library_suffix} "
      . join(" ", @library_targets)
      . " \$(LIBS)\n");
} else {
print(OUT
"	\$(AR) rcs -o src/${library_prefix}Mw.a "
      . join(" ", @library_targets)
      . " \$(LIBS)\n");
}

foreach my $l (@library_targets) {
    my $warn = "-Wall -Wextra -Wno-sign-compare";
    my $s    = $l;
    my $o    = $object_suffix;
    $o =~ s/\./\\\./g;
    $s =~ s/$o$/.c/;

    if ($l =~ /^external\//) {
        $warn = "";
    }

    print(OUT "${l}: ${s}\n");
    print(OUT "	\$(CC) $warn \$(CFLAGS\) \$\(INCDIR) -c -o ${l} ${s}\n");
}
print(OUT "\n");
print(OUT "\n");
print(OUT "examples: " . join(" ", @examples_targets) . "\n");
print(OUT "\n");
foreach my $l (@examples_targets) {
    my $libs = "";
    my $s    = $l;
    my $o    = $executable_suffix;
    $o =~ s/\./\\\./g;
    $s =~ s/$o$//;

    if (defined($examples_libs{$l})) {
        $libs = $examples_libs{$l};
    }

    print(OUT
"${l}: ${s}${object_suffix} src/${library_prefix}Mw${library_suffix}\n"
    );
    print(OUT
"	\$(CC) -L src -Wl,-R./src \$\(CFLAGS) \$\(LIBDIR) -o ${l} ${s}${object_suffix} ${example_mw_link} ${math} ${libs}\n"
    );
    print(OUT "${s}${object_suffix}: ${s}.c\n");
    print(OUT
          "	\$(CC) -c \$\(CFLAGS) \$\(INCDIR) -o ${s}${object_suffix} ${s}.c ${example_mw_link} ${math}\n"
    );
}
print(OUT "\n");
print(OUT "clean:\n");
print(OUT
"	rm -f */*.o */*/*.o */*/*/*.o */*.exe */*/*.exe */*/*/*.exe src/*.so src/*.dll src/*.a "
      . join(" ", @examples_targets)
      . "\n");
print(OUT "\n");
print(OUT "distclean: clean\n");
print(OUT "	rm -f Makefile\n");
close(OUT);
