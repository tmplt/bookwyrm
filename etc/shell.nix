with import <nixpkgs> {};
mkShell {
  # this adds all the build inputs of your project package
  inputsFrom = [ (import ./default.nix { inherit stdenv cmake python36 python36Packages curl ncurses; }) ];
  buildInputs = with pkgs; [ clang-tools cppcheck python36Packages.pygments ];
}
