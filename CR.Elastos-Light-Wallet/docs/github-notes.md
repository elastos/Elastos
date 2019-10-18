### to force only one version of elastos-light-wallet

  rm -rf .git;
  git init;
  find . -exec touch {} \;
  git add .;
  git commit -m "Initial commit";
  git remote add origin https://github.com/coranos/elastos-light-wallet.git;
  git push -u --force origin master;
  git branch --set-upstream-to=origin/master master;
  git pull;git push;

## to delete release tags

  git push --delete origin 1.0.0-RC6;
  git tag -d 1.0.0-RC6;
  git pull;git push;

## new release template:
  1.0.0-RC00

  windows:

  certUtil -hashfile dist/elastos-light-wallet\ Setup\ 1.0.0-RC.exe sha256

  xxxx

  Ubuntu 18:

  openssl sha -sha256 dist/elastos-light-wallet-1.0.0-RC.AppImage

  xxxx

  Mac:

  openssl dgst -sha256 dist/elastos-light-wallet-1.0.0-RC.dmg
  
  xxxx
