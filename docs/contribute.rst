Contributing
==============

We’re excited to work with you! Post in the issues queue with any questions, feature requests, or proposals.

Automated Testing
--------------------

This module uses `Tripal Test Suite <https://tripaltestsuite.readthedocs.io/en/latest/installation.html#joining-an-existing-project>`_. To run tests locally:

.. code:: bash

  cd MODULE_ROOT
  composer up
  ./vendor/bin/phpunit

This will run all tests associated with the ND Genotypes extension module. If you are running into issues, this is a good way to rule out a system incompatibility.

.. warning::

  It is highly suggested you ONLY RUN TESTS ON DEVELOPMENT SITES. We have done our best to ensure that our tests clean up after themselves; however, we do not guarantee there will be no changes to your database.

.. _demo-instructions:

Manual Testing (Demonstration)
--------------------------------

We have provided a `Tripal Test Suite Database Seeder <https://tripaltestsuite.readthedocs.io/en/latest/db-seeders.html>` to make development and demonstration of functionality easier. To populate your development database with fake phenotypic data:

1. Install this module according to the instructions in the administration guide.
2. Create an organism (genus: Tripalus; species: databasica)
3. Run the database seeder to populate the database using the following commands:

  .. code::

    cd MODULE_ROOT
    composer up
    ./vendor/bin/tripaltest db:seed GenotypeDatasetSeeder

4. Populate the materialized views by going to Administration » Tripal » Extensions » Natural Diversity Genotypes » Sync and Choose "Tripalus" then click the "Sync" button. Finally run the Tripal jobs submitted.
5. To play with the genotype matrix go to ``[your drupal site]/chado/genotype/[genus]``. You can see what germplasm are available by typing a single random letter in the autocomplete box.
6. To play with marker/variant pages, go to Administration » Content » Tripal Content » Publish Tripal Content and then select "Genetic Marker"/"Sequence Variant" and publish to create pages. Remember to run the tripal jobs submitted.

.. warning::

  NEVER run database seeders on production sites. They will insert fictitious data into Chado.

.. warning::

  If ND Genotypes fields are not automatically attached to the genetic marker and sequence variant content types, go to the "Manage Fields" page for each and click "Find new fields". Also, go to the "Manage Display" page and ensure they are not hidden.
