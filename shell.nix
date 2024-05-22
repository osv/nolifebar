{ pkgs ? import <nixpkgs> {} }:

with pkgs;

mkShell {
  buildInputs = [
    autoconf
    automake
    gcc
    xorg.libX11.dev
    xorg.libXft
    pkg-config
  ];
}
