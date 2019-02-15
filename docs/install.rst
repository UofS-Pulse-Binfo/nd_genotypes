
Installation
============

Quickstart
-----------
1. Install the following dependencies: Tripal 3.x and Tripal Download API. Also, ensure you have PostgreSQL 9.3+.
2. Install this module as you would any Drupal module (ie: download, unpack in ``sites/all/modules`` and enable through ``http://[your site]/admin/modules``)
3. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes.
4. Load data using the `genotype loader <https://github.com/UofS-Pulse-Binfo/genotypes_loader>`_. Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site.

- You can access the genotype matrix at ``[your drupal site]/chado/genotypes/[genus]``.
- You should see a "Genotypes" and updated "Sequences" pane on Genetic Marker and Variant pages.

  - You may need to go to Administration > Structure > Tripal Content Types > Genetic Marker > Manage Fields and click "Find new fields".
  - Then go to "Manage Display" and enable the field by dragging it into the display area.

Dependencies
------------

1. [Tripal 3.x](https://drupal.org/project/tripal)
2. [Tripal Download API](https://github.com/tripal/trpdownload_api)
3. PostgreSQL 9.3 (9.4+ recommended)

Installation
-------------

This module is available as a project on Drupal.org. As such, the preferred method of installation is using Drush:

.. code:: bash

  cd [your drupal root]/sites/all/modules
  git clone https://github.com/tripal/trpdownload_api.git
  git clone https://github.com/UofS-Pulse-Binfo/nd_genotypes.git

The above command downloads the module into the expected directory (e.g. /var/www/html/sites/all/modules/nd_genotypes). Next we need to install the module:

.. code:: bash

  drush pm-enable nd_genotypes

Now that the module is installed, we just need to configure it!
