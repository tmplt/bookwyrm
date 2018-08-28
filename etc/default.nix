{ stdenv, cmake, python36, python36Packages, curl, ncurses }:
with python36Packages;

let
  homepagePrefix = "https://pypi.org/project";

  # TODO: PR these to nixpkgs
  orderedmultidict = buildPythonPackage rec {
    name = "${pname}-${version}";
    pname = "orderedmultidict";
    version = "0.7.11";

    src = fetchPypi {
      inherit pname version;
      sha256 = "dc2320ca694d90dca4ecc8b9c5fdf71ca61d6c079d6feb085ef8d41585419a36";
    };

    propagatedBuildInputs = [ six ];

    meta = with stdenv.lib; {
      description = "Ordered Multivalue Dictionary";
      homepage = "${homepagePrefix}/${pname}";
      license = licenses.unlicense;
    };
  };

  furl = buildPythonPackage rec {
    name = "${pname}-${version}";
    pname = "furl";
    version = "1.0.1";

    src = fetchPypi {
      inherit pname version;
      sha256 = "6bb7d9ed238a0104db3a638307be7abc35ec989d883f5882e01cb000b9bdbc32";
    };

    propagatedBuildInputs = [ orderedmultidict ];

    doCheck = false;

    meta = with stdenv.lib; {
      description = "URL parsing and manipulation made easy";
      homepage = "${homepagePrefix}/${pname}";
      license = licenses.unlicense;
    };
  };

  isbnlib = buildPythonPackage rec {
    name = "${pname}-${version}";
    pname = "isbnlib";
    version = "3.8.4";

    src = fetchPypi {
      inherit pname version;
      sha256 = "76b978410874b140bd2622bb2b7658d90ae5794baf4dea9507b05a4b68ee3bfc";
    };

    # Not critical; no need to test
    doCheck = false;

    meta = with stdenv.lib; {
      description = "Extract, clean, transform, hyphenate and metadata for ISBNs (International Standard Book Number)";
      homepage = "${homepagePrefix}/${pname}";
      license = licenses.lgpl3;
    };
  };
in
  stdenv.mkDerivation rec {
    name = "bookwyrm-${version}";
    version = "0.5.0";

    src = ../.;

    meta = with stdenv.lib; {
      description = "A TUI for searching for and downloading publicly available ebooks";
      license = licenses.mit;
      maintainers = [];
      platforms = platforms.linux;
    };

    buildInputs = [
      curl
      ncurses
      python36

      (python36.buildEnv.override {
        extraLibs = [
          beautifulsoup4
          furl
          requests
          isbnlib
        ];
      })
    ];

    nativeBuildInputs = [ cmake ];
  }
