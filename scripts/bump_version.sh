#!/bin/bash

# Read the current version from /ices.info.
# Bump the version number.
# Write the new version number to /ices.info and all .info files in the ces_* folders.


# Read the current version from /ices.info.
current_version=$(grep 'version = ' ices.info | sed 's/version = //')

# Bump the version number.
echo "Current version: $current_version"
echo "Bumping version..."
new_version=$(echo "$current_version" | awk -F. '{$NF = $NF + 1;} 1' | sed 's/ /./g')


# Write the new version number to /ices.info and all .info files in the ces_* folters.
sed -i "s/version = $current_version/version = $new_version/" ices.info
find ces_* -name '*.info' -exec sed -i "s/version = $current_version/version = $new_version/" {} \;
echo "New version: $new_version"
echo "Updated /ices.info with new version."
echo "Updated all .info files in ces_* folders with new version."

# remove enclosing quotes from new_version
new_version=$(echo $new_version | sed 's/"//g')
# commit all the changes
git add ices.info ces_*/*.info
git commit -m "Bump version $new_version"
echo "Committed changes to git."
# create a git tag
git tag -a "$new_version" -m "Bump version $new_version"
echo "Created git tag $new_version"


