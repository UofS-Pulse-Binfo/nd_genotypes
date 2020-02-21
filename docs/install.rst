
Installation
============

Quickstart
-----------
1. Install the following dependencies: Tripal 3.x and Tripal Download API. Also, ensure you have PostgreSQL 9.3+.
2. Install this module as you would any Drupal module (ie: download, unpack in ``sites/all/modules`` and enable through ``http://[your site]/admin/modules``)
3. Load data using the `genotype loader <https://github.com/UofS-Pulse-Binfo/genotypes_loader>`_. Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site. Once data is available make sure to sync it (Administration » Tripal » Extensions » Natural Diversity Genotypes » Sync)
4. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes » Settings.

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
2. `Tripal Download API <https://github.com/tripal/trpdownload_api>`_
3. `Tripal D3.js <https://github.com/tripal/tripald3>`_
4. PostgreSQL 9.3 (9.4+ recommended; tested with 11.3)

Installation
-------------

This module is available as a project on Drupal.org. As such, the preferred method of installation is using Drush:

.. code:: bash

  cd [your drupal root]/sites/all/modules
  git clone https://github.com/tripal/trpdownload_api.git
  git clone https://github.com/tripal/tripald3.git
  git clone https://github.com/UofS-Pulse-Binfo/nd_genotypes.git

The above command downloads the module into the expected directory (e.g. /var/www/html/sites/all/modules/nd_genotypes). Next we need to install the module:

.. code:: bash

  drush pm-enable libraries trpdownload_api
  drush pm-enable tripald3 nd_genotypes

Now that the module is installed, we just need to configure it!
