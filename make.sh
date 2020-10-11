# create dist if not exists
if [ ! -d "dist/" ]; then
  mkdir dist
fi
# copy settings if not exists
if [ ! -f "dist/settings.ini" ]; then
  cp settings.ini dist/settings.ini
fi

# build
cd src
make
