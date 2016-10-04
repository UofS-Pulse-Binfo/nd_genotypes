# Natural Diversity Genotypes
This module provides a more specialized interface to genotypic information
stored in the chado natural diversity module tables including adding summaries
of this information to feature pages and providing a powerful marker by germplasm
matrix.

## Not yet Production-Ready
This module is currently under-going a complete re-birth as it switches from using Drupal Views and moves towards a much more efficient framework. This process was initiated to ensure this modue can handle large datasets with a testing goal of 5 billion genotype calls (5 million variants by 1000 germplasm). It is currently a fully PostgreSQL solution although it is being developed to allow use of other data backends in the future as needed.

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

### Marker/Variant Pages
This module adds two panes to marker and variant pages:

1. A "Genotypes" pane which summarizes the calls recorded for a given marker/variant. Specifically, a pie chart depicts the ratio of calls recorded.
2. A "Sequences" pane which shows 500bp flanking sequence for the current marker/variant with the alleles for the current marker highlighted as [A/B] and all other variants located in the flanking sequence indicated using their IUPAC codes. This display provides an easy means for researchers to design new markers if the old technology is no longer desirable.

