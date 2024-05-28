{ pkgs ? import <nixpkgs> {} }:

with pkgs;

mkShell {
  buildInputs = [
    autoconf
    automake
    gcc
    xorg.libX11.dev
    xorg.libXft

    xorg.libxcb.dev
    xorg.xcbutil.dev
    xorg.xcbutilwm.dev

    xorg.xcbutilerrors
    xorg.xcbutilrenderutil

    pkg-config
  ];
}
