{
  description = "Flake for nolifebar";

  inputs = {
    nixpkgs = {
      url = "github:NixOS/nixpkgs/nixos-23.11";
    };
  };

  outputs = { self, nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];

      # Define the overlay to override dzen2
      overlay = final: prev: {
        dzen2 = prev.dzen2.overrideAttrs (oldAttrs: rec {
          src = prev.fetchFromGitHub {
            owner = "osv";
            repo = "dzen";
            rev = "8d7d7d0";
            sha256 = "sha256-Bm3CK/iNG1P8i88C3AougTz3y+QzkGr7ptp1S50OdlU=";
          };
        });
      };

    in
      rec {
        packages = builtins.listToAttrs (map (system: {
          name = system;
          value = let
            pkgs = import nixpkgs {
              system = system;
              overlays = [ overlay ];  # Apply the overlay
            };
          in
            pkgs.stdenv.mkDerivation rec {
              pname = "nolifebar";
              version = "0.9";

              src = self;
              nativeBuildInputs = [
                pkgs.autoreconfHook  # Use autoreconfHook instead of manual autoreconf
                pkgs.pkg-config      # Ensure pkg-config is in nativeBuildInputs
              ];

              buildInputs = with pkgs; [
                autoconf
                automake
                gcc

                xorg.libX11.dev
                xorg.libXft
                xorg.libxcb.dev
                xorg.xcbutil.dev
                xorg.xcbutilwm.dev
                libpulseaudio.dev
                dzen2
              ];

              preConfigure = ''
                autoreconf -vfi
              '';

              meta = with pkgs.lib; {
                description = "A small X11 utility that hides the life bar in a game";
                license = licenses.wtfpl;
                homepage = "https://github.com/osv/nolifebar";
                maintainers = [ maintainers.yourname ];
                platforms = platforms.unix;
              };
            };
        }) systems);

        # Set the default package for the current system
        defaultPackage.x86_64-linux = packages.x86_64-linux;
        defaultPackage.aarch64-linux = packages.aarch64-linux;

        # Add the apps attribute without using mkApp
        apps = builtins.listToAttrs (map (system: {
          name = system;
          value = let
            pkg = packages.${system};
          in
            {
              nolifebar-start = {
                type = "app";
                program = "${pkg}/bin/nolifebar-start";
              };
            };
        }) systems);

        devShell = builtins.listToAttrs (map (system: {
          name = system;
          value = let
            pkgs = import nixpkgs {
              system = system;
              overlays = [ overlay ];  # Apply the overlay
            };
          in
            pkgs.mkShell {
              buildInputs = with pkgs; [
                autoconf
                automake
                gcc

                xorg.libX11.dev
                xorg.libXft
                xorg.libxcb.dev
                xorg.xcbutil.dev
                xorg.xcbutilwm.dev
                libpulseaudio.dev

                xkb-switch
                pkg-config

                # Runtimes for plugins
                bc
                cava
                pulseaudio  # pactl
                wirelesstools  # iwgetid, iwevent

                valgrind

                dzen2  # Include the overridden dzen2 in the devShell
              ];

              shellHook = ''
                echo "!! Entering the nolifebar development shell"
              '';
            };
        }) systems);
      };
}
