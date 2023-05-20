# This is a debian bullseye with php 8.0 and apache.
FROM drupal:7

# Configure apache: change port from 80 to 2029.
# We ned to change the port so from inside the container the url localhost:2029 is accessible 
# and hence drupal can access himself.
