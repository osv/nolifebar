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

    # runrimes for plugins
    bc
    cava
    pulseaudio # pactl

    (dzen2.overrideAttrs (oldAttrs: rec {
      patchPhase = ''
        ${oldAttrs.patchPhase or ""}
        sed -i 's/MAX_LINE_LEN[[:space:]]*8192/MAX_LINE_LEN   5 * 8192/' dzen.h
      '';
    }))
  ];

  shellHook = ''
    echo "Welcome to the development shell"
  '';
}
