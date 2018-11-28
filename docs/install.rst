
Installation
============

Install this module as you would any other Drupal module after ensuring you have the following dependencies.

Dependencies
------------

1. Tripal 3.x
2. Tripal Download API
3. PostgreSQL 9.3 (9.4+ recommended)

Usage
------

1. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes.
2. Load data using the `genotype loader <https://github.com/UofS-Pulse-Binfo/genotypes_loader>`_. Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site.
3. You can access the genotype matrix at ``[your drupal site]/chado/genotypes/[genus]``.
4. You should see a "Genotypes" and updated "Sequences" pane on Genetic Marker and Variant pages.