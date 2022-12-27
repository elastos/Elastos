### to force only one version of elastos-light-wallet

  rm -rf .git;
  git init;
  find . -exec touch {} \;
  git add .;
  git commit -m "Initial commit";
  git remote add origin https://github.com/cyber-republic/elastos-light-wallet.git;
  git push -u --force origin master;
  git branch --set-upstream-to=origin/master master;
  git pull;git push;

## to delete release tags

  git push --delete origin 1.0.0-RC6;
  git tag -d 1.0.0-RC6;
  git pull;git push;
