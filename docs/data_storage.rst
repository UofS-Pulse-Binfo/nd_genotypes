
Data Storage
============

This module stores most of it's data in official chado tables with only genotypes being in a well linked chado-like table (`genotype_call` table). We went with this backwards compatible approach to make supporting large genotypic datasets more efficient then chado alone. For more information on our schema and the reasons we went with this approach see :doc:`our schema documentation </data_storage/schema>`.

.. toctree::
   :maxdepth: 2

   data_storage/schema
   data_storage/benchmarking
   data_storage/exampledb
