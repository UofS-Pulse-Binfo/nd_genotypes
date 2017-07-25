# Natural Diversity Genotypes
This module provides support and visualization of genotypic data stored in a modified GMOD Chado schema. The 3.x branch of this module represents a shift towards support for **large scale genotypic datasets** through backwards compatible improvements to the Chado schema including a new gathering table for genotypes (genotype_call) modelled after the chado phenotype table and modern PostgreSQL indices.

## Dependencies
1. Tripal 3.x
2. Tripal Download API
3. PostgreSQL 9.3 (9.4 recommended)

## Installation
1. Install the dependencies listed above.
2. Install this module as you would any other Drupal module.

## Usage
1. Configure this module by going to Administration » Tripal » Extensions » Natural Diversity Genotypes. Specifically, you need to set the various controlled vocabulary terms that this module will expect.
2. Load data using the [genotype loader](https://github.com/UofS-Pulse-Binfo/genotypes_loader). Since the Genotype loader is not yet released, we highly suggest test loading each dataset on a development site.
3. You can access the genotype matrix at [your drupal site]/chado/genotypes/[genus].
4. You should see a "Genotypes" and updated "Sequences" pane on Marker and Variant pages.

## Functionality
1. Configuration allowing admin to specify controlled vocabulary terms used for marker and variant types, as well as, relationship types.
2. Genotype matrix: A table-based search allowing users to retrieve a specific variant by germplasm slice of genotype data
3. A genotype summary per marker and variant shown as it's own pane on marker and variant feature pages.
4. A variant marked-up sequence per marker and variant providing the user with 500bp flanking sequence with flanking variants highlighted via linked IUPAC codes.
5. Custom form element types providing an interface for users to select multiple germplasm and choose a pairwise combination of germplasm.
6. Sync functionality allowing admin to cache genotype calls and speed up the afore mentioned functionality.
7. Countless efficiency improvements including optimized queries and well-choosen indexes.

### Genotype Matrix
This module provides genotype search functionality that allows users to select which germplasm and variants they are interested in and be shown a colour-coded variant by germplasm table which can be further filtered by marker/variant type and to only show polymorphic variants (pairwise comparison choosen by the user). After filtering to their desired dataset, the user can download the table as a tab-delimited file.

The following screenshot shows the form the user is presented with for choosing filter criteria:
![genotypematrix_form](https://cloud.githubusercontent.com/assets/1566301/19090330/1d656d6c-8a3b-11e6-8776-6f3c1e10e18b.png)

The following screenshot shows the table of genotype calls presented to the user based on their filter criteria:
![genotypematrix_table](https://cloud.githubusercontent.com/assets/1566301/19090346/2e83e0b0-8a3b-11e6-9ad3-9574aa88b7e5.png)

### Marker/Variant Pages
This module adds two panes to marker and variant pages:

1. A "Genotypes" pane which summarizes the calls recorded for a given marker/variant. Specifically, a pie chart depicts the ratio of calls recorded.
2. A "Sequences" pane which shows 500bp flanking sequence for the current marker/variant with the alleles for the current marker highlighted as [A/B] and all other variants located in the flanking sequence indicated using their IUPAC codes. This display provides an easy means for researchers to design new markers if the old technology is no longer desirable.
