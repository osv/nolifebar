{
  description = "nolifebar status bar";

  inputs = {
    nixpkgs = {
      url =
        "github:NixOS/nixpkgs/51063ed4"; # temporary pin nixos-23.11 because of cava segfault
    };
  };

  outputs = { self, nixpkgs, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];

      myDzen2Fork = pkgs:
        pkgs.dzen2.overrideAttrs (oldAttrs: rec {
          src = pkgs.fetchFromGitHub {
            owner = "osv";
            repo = "dzen";
            rev = "8d7d7d0";
            sha256 = "sha256-Bm3CK/iNG1P8i88C3AougTz3y+QzkGr7ptp1S50OdlU=";
          };
        });

    in rec {
      defaultOptions = {
        useOSVDzen2Fork = true; # The original dzen2 is too slow and does not support enough large pipe data.
        useCava = true;
        usePulseaudio = true;
        useWirelessTools = true;
      };

      # Function to create packages with options
      packageWithOptions = { options ? { } }:
        builtins.listToAttrs (map (system: {
          name = system;
          value = let
            pkgs = import nixpkgs { inherit system; };
            lib = pkgs.lib;
            actualOptions = lib.recursiveUpdate defaultOptions options;
            runtimeDeps = with pkgs;
              [ bash bc xkb-switch ]
              ++ lib.optional actualOptions.useOSVDzen2Fork (myDzen2Fork pkgs)
              ++ lib.optional actualOptions.useCava cava
              ++ lib.optional actualOptions.usePulseaudio pulseaudio
              ++ lib.optional actualOptions.useWirelessTools wirelesstools;
          in pkgs.stdenv.mkDerivation rec {
            pname = "nolifebar";
            version = "1.0";

            src = self;

            nativeBuildInputs =
              [ pkgs.autoreconfHook pkgs.pkg-config pkgs.makeWrapper ];

            buildInputs = with pkgs;
              [
                autoconf
                automake
                gcc

                xorg.libX11.dev
                xorg.libXft
                xorg.libxcb.dev
                xorg.xcbutil.dev
                xorg.xcbutilwm.dev
              ]
              ++ lib.optional actualOptions.usePulseaudio libpulseaudio.dev;

            preConfigure = ''
              autoreconf -vfi
            '';

            postInstall = ''
              wrapProgram $out/bin/nolifebar-start \
                --prefix PATH : ${lib.makeBinPath runtimeDeps}
            '';

            meta = with pkgs.lib; {
              description =
                "A Unix way and easy-to-use tool for generating status bars (dzen2, lemonbar) ";
              license = licenses.wtfpl;
              homepage = "https://github.com/osv/nolifebar";
              platforms = platforms.unix;
            };
          };
        }) systems);

      # Define packages as attribute sets containing derivations
      packages = builtins.listToAttrs (map (system: {
        name = system;
        value = {
          default = (packageWithOptions { }).${system};
          # Expose packageWithOptions for custom options
          packageWithOptions = packageWithOptions;
        };
      }) systems);

      # Set the default package for the current system
      defaultPackage =
        builtins.mapAttrs (system: pkgSet: pkgSet.default) packages;

      # Add the apps attribute
      apps = builtins.listToAttrs (map (system: {
        name = system;
        value = {
          nolifebar-start = {
            type = "app";
            program = "${packages.${system}.default}/bin/nolifebar-start";
          };
        };
      }) systems);

      devShell = builtins.listToAttrs (map (system: {
        name = system;
        value = let pkgs = import nixpkgs { inherit system; };
        in pkgs.mkShell {
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

            pkg-config

            # Runtimes for plugins
            xkb-switch
            bc
            cava
            pulseaudio # pactl
            wirelesstools # iwgetid, iwevent
            xorg.xdpyinfo.out   # for dzen2 popup
            valgrind

            (myDzen2Fork pkgs) # Include the overridden dzen2 in the devShell
          ];

          shellHook = ''
            echo "!! Entering the nolifebar development shell"
          '';
        };
      }) systems);
    };
}
