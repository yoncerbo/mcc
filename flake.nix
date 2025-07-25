{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };
  outputs = { self, nixpkgs, }:
  let 
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {
    devShell."${system}" = pkgs.mkShell rec {
      packages = with pkgs; [
        linuxHeaders
        bear
        gnumake
      ];
    };
  };
}

