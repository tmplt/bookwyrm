with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "bookwyrm-env";
  buildInputs = [
    cmake
    curlFull
    gcc7
    python36Full
  ];
}
