
Data Storage
============

Genotypic data is stored through use of a custom table (genotype_call) created by this module. This table provides a centralized, relational table which pulls all the information for a given genotypic call (marker assay result on a given germplasm for a specific project) together in a single record. It also supports flexible storage for all meta-data associated with a genotype assay result through a PostgreSQL JSONB metadata column. We went with this backwards compatible approach to make supporting large genotypic datasets more efficient then chado alone. For more information on our schema and the reasons we went with this approach see :doc:`our schema documentation </data_storage/schema>`.

.. toctree::
   :maxdepth: 2

   data_storage/schema
   data_storage/benchmarking
   data_storage/exampledb
