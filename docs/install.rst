
Installation
============

Quickstart
-----------
This installation assumes you have Tripal 3.x and PostgreSQL 9.3+.

1. Install the following dependencies: Drupal Libraries API, Tripal D3.js, Tripal Donwload API.

.. code::

  drush pm-download libraries
  drush pm-enable libraries -y
  cd [drupal root]/sites/all/modules
  git clone https://github.com/tripal/tripald3
  git clone https://github.com/tripal/trpdownload_api
  cd [drupal root]/sites/all/libraries
  mkdir d3 && cd d3
  wget https://github.com/d3/d3/releases/download/v3.5.17/d3.zip
  unzip d3.zip
  drush pm-enable trpdownload_api tripald3 -y

2. Install this module as you would any Drupal module.

.. code::

  cd [drupal root]/sites/all/modules
  git clone https://github.com/UofS-Pulse-Binfo/nd_genotypes.git
  drush en nd_genotypes -y

3. Load data using the `genotype loader <https://github.com/UofS-Pulse-Binfo/genotypes_loader>`_. Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site.
4. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes » Settings.
5. Once data is available make sure to sync it (Administration » Tripal » Extensions » Natural Diversity Genotypes » Sync)

.. note::

  If you do not have data and you want to try the module out, you can use the Tripal Test Suite Database Seeder provided with this module. See :ref:`demo-instructions`.

- You can access the genotype matrix at ``[your drupal site]/chado/genotype/[genus]``.
- You should see a "Genotypes" and updated "Sequences" pane on Genetic Marker and Variant pages.

  - You may need to go to Administration > Structure > Tripal Content Types > Genetic Marker > Manage Fields and click "Find new fields".
  - Then go to "Manage Display" and enable the field by dragging it into the display area.

.. note::

  If ND Genotypes fields are not automatically attached to the genetic marker and sequence variant content types, go to the "Manage Fields" page for each and click "Find new fields". Also, go to the "Manage Display" page and ensure they are not hidden.

Dependencies
------------

1. `Tripal 3.x <https://drupal.org/project/tripal>`_
2. `Drupal Libraries API <https://www.drupal.org/project/libraries>`_
3. `Tripal Download API <https://github.com/tripal/trpdownload_api>`_
4. `Tripal D3.js <https://github.com/tripal/tripald3>`_
5. PostgreSQL 9.3 (9.4+ recommended; tested with 11.3)

Installation
-------------

1. Install the following dependencies: Drupal Libraries API, Tripal D3.js, Tripal Donwload API.

    - First we install the Drupal Libraries API which is required for Tripal D3.

    .. code::

      drush pm-download libraries
      drush pm-enable libraries -y

    -  Next we grab the latest version of the remaining dependencies from Github.

    .. code::

      cd [drupal root]/sites/all/modules
      git clone https://github.com/tripal/tripald3
      git clone https://github.com/tripal/trpdownload_api

    - The charts for the module are drawn using D3.js v3 . As such we need to download it and place it in our libraries folder.

    .. code::

      cd [drupal root]/sites/all/libraries
      mkdir d3 && cd d3
      wget https://github.com/d3/d3/releases/download/v3.5.17/d3.zip
      unzip d3.zip

    - Finally we can enable the last of our dependencies.

    .. code::

      drush pm-enable trpdownload_api tripald3 -y

2. Install this module as you would any Drupal module.

.. code::

  cd [drupal root]/sites/all/modules
  git clone https://github.com/UofS-Pulse-Binfo/nd_genotypes.git
  drush en nd_genotypes -y
