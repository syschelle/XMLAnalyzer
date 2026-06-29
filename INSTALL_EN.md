# Installation Guide (Docker)

This guide explains how to run ConfigScope on an external system.

## Requirements
- Docker Engine / Docker Desktop
- Docker Compose v2 (`docker compose`)
- For the GitHub workflow: GitHub Actions with package write permissions enabled

## API key (recommended)
Set `API_KEY` directly in `docker-compose.yml` or `docker-compose.image.yml`.
Use the same value in request header `X-API-Key`.
If `API_KEY` is empty or missing, API access is open.

No `.env` file is required.

## ScriptAccess Code feature flags
Set the ScriptAccess options directly in the Compose file.

```yaml
SCRIPT_ACCESS_CODE_ENABLED: "true"       # show and enable ScriptAccess Code
SCRIPT_ACCESS_CODE_ENABLED: "false"      # hide and disable ScriptAccess Code
SCRIPT_ACCESS_CODE_API_ENABLED: "true"   # allow the runtime API toggle
SCRIPT_ACCESS_CODE_API_ENABLED: "false"  # disable the runtime API toggle
```

`SCRIPT_ACCESS_CODE_ENABLED` controls the UI/button and `/script_access_code` endpoints.
`SCRIPT_ACCESS_CODE_API_ENABLED` controls the API endpoints used to read or change the ScriptAccess Code status at runtime.

After changing the Compose file, recreate the container:

```bash
docker compose -f docker-compose.image.yml up -d --force-recreate
```

Runtime API toggle, protected by `X-API-Key` when `API_KEY` is configured and only available when `SCRIPT_ACCESS_CODE_API_ENABLED` is `"true"`:

```bash
curl -H "X-API-Key: <your-key>" http://localhost:18080/api/v1/script-access-code/status
curl -X POST http://localhost:18080/api/v1/script-access-code/status \
  -H "X-API-Key: <your-key>" \
  -H "Content-Type: application/json" \
  -d '{"enabled": false}'
```

Runtime API changes are reset to the Docker Compose `SCRIPT_ACCESS_CODE_ENABLED` value after container restart. If `SCRIPT_ACCESS_CODE_API_ENABLED` is `"false"`, the runtime API endpoints return `404`.

## Deploy Download feature flag
Set the Deploy Download option directly in the Compose file.

```yaml
DEPLOY_DOWNLOAD_ENABLED: "true"   # show and enable Deploy Download
DEPLOY_DOWNLOAD_ENABLED: "false"  # hide and disable Deploy Download
```

When disabled, the Deploy Download button is hidden and `/download_bundle` returns `404`.
After changing the Compose file, recreate the container.

## Local start with build from source
Use this variant when Docker should build the image locally from `webapp/Dockerfile`.

```bash
docker compose up -d --build
```

## Start with prebuilt GitHub Container Registry image
Use this variant when the image was built by GitHub Actions and pushed to GitHub Container Registry.

```bash
docker compose -f docker-compose.image.yml up -d
```

Image used by `docker-compose.image.yml`:

```text
ghcr.io/syschelle/export-xml-web:latest
```

## GitHub Actions image build
The workflow is stored here:

```text
.github/workflows/docker-images.yml
```

It builds the Docker image from:

```text
webapp/Dockerfile
```

The workflow pushes these tags to GitHub Container Registry:

- `v0.163`
- `sha-<short-sha>`
- `latest` for the current published image
- the Git tag name when a `v*` tag is pushed

`docker-compose.image.yml` uses `ghcr.io/syschelle/export-xml-web:latest` by default. Edit the `image:` line if you want to pin a fixed version tag.

## Open
- http://localhost:18080
- or http://<SERVER-IP>:18080

## API example
```bash
curl -X POST "http://localhost:18080/api/v1/export-xml/analyze" \
  -H "X-API-Key: <your-key>" \
  -F "file=@/path/to/export.xml;type=application/xml"
```


## Data handling and demo disclaimer
ConfigScope is intended only as a demonstration and analysis aid. Use is at the user's own risk.

Uploaded XML, license and performance CSV files are processed in memory for the current request. The application does not intentionally write uploaded files, parsed analysis results or generated report data to disk, a database or persistent server-side storage. After the response has been rendered, the displayed analysis exists in the browser only.

Exports such as Excel, PDF/print output, AGH/AGD files and the deploy bundle are generated on demand and are stored only where the user explicitly saves or downloads them. Standard infrastructure such as browser caches, reverse proxies, web server access logs or container/platform logging can still record technical metadata such as request paths, timestamps or client addresses depending on the deployment environment.

## Stop
```bash
docker compose down
# or, for the image based compose file:
docker compose -f docker-compose.image.yml down
```


## ScriptAccess UI

When enabled, ScriptAccess Code is not shown as a standalone button. It is intentionally hidden behind the letter `n` in the `ConfigScope` title.




## Complete Performance PDF report

For Performance CSV results, ConfigScope provides a dedicated complete PDF report button. The report is generated through the browser print/PDF dialog and includes the complete performance tables plus the related charts. For two-file comparisons, the report also includes the raw detail rows for both CSV files. This is available for both a single Performance CSV and the two-file Performance CSV comparison.

The complete report is designed for sharing the full analysis with customers. It does not create a server-side PDF file; the file only exists where the user explicitly saves it from the browser.

## Performance CSV column help

Performance CSV tables include hover explanations on column headers. Move the mouse over a header to see what the metric means, including timing values, percentile values, slow-measurement counts and comparison deltas.

## Performance CSV comparison

The web upload supports selecting up to two files.
A comparison view is available only when both selected files are Performance CSV files. The comparison includes tables and modality-based comparison charts with PDF export. ConfigScope derives comparison labels from the differing parts of the two filenames and uses those labels instead of generic A/B labels. The differing part may be at the beginning, in the middle or at the end before the `.csv` extension. The chart section is shown after the comparison tables.
`export.xml` and license XML files do not support two-file comparison and must be analyzed individually.


## Performance CSV missing columns

In the two-file Performance CSV comparison, missing expected columns are reported per file.
The warning specifies which selected CSV file is missing which columns, instead of showing only one combined list.
