Use grpc with nginx: https://medium.com/@fzambia/serving-secure-websocket-and-grpc-over-same-port-with-nginx-659662a18368
Secure nginx with lets encrypt: https://www.digitalocean.com/community/tutorials/how-to-secure-nginx-with-let-s-encrypt-on-ubuntu-18-04
GRPC with SSL: https://github.com/sandtable/ssl_grpc_example
GRPC with nginx: https://www.nginx.com/blog/nginx-1-13-10-grpc/?utm_campaign=core&utm_medium=blog&utm_source=youtube&utm_content=grpc

Ethereum Sidechain:
- Install solidity compiler: https://solidity.readthedocs.io/en/latest/installing-solidity.html#binary-packages
- 

Google Cloud Engine:
- Getting started with grpc on Compute Engine:  https://cloud.google.com/endpoints/docs/grpc/get-started-compute-engine-docker
- loadbalancing with instance groups: https://medium.com/google-cloud/grpc-l7-loadbalancing-with-container-optimized-managed-instance-groups-4a68ba4ff8e8?
- gcloud  create-with-container: https://cloud.google.com/sdk/gcloud/reference/beta/compute/instances/create-with-container
- Creating and using ssl certs: https://cloud.google.com/load-balancing/docs/ssl-certificates


Setup in GCE:

## One time
gcloud compute addresses create gce-ingress --global;
gcloud compute addresses describe  gce-ingress --global --format="value(address)";
gcloud beta compute ssl-certificates create elastos-smartweb-service-certificate \
    --domains elastos-smartweb-service.nucleusconsole.com --description "Certificate for Elastos Smartweb Service";

## Repeat
gcloud beta compute instance-templates create-with-container elastos-smartweb-service-template \
    --machine-type g1-small --tags elastos-smartweb-service-server \
    --container-image="cyberrepublic/elastos-smartweb-service" --container-env-file=.env;
gcloud compute instance-groups managed create elastos-smartweb-service-group \
    --base-instance-name elastos-smartweb-service --zone us-east4-c --size 2 \
    --template elastos-smartweb-service-template;
gcloud compute firewall-rules create firewall-rules-grpc --allow=tcp:8001 \
    --source-ranges=130.211.0.0/22,35.191.0.0/16 --target-tags=elastos-smartweb-service-server;
gcloud compute instance-groups managed set-named-ports elastos-smartweb-service-group \
    --named-ports=grpc-port:8001 --zone us-east4-c;
# Create health check with name "elastos-smartweb-service-health-check"
gcloud beta compute backend-services create elastos-smartweb-service-backend-service \
    --port-name=grpc-port --protocol=http2 --health-checks=elastos-smartweb-service-health-check --global;
gcloud beta compute backend-services add-backend elastos-smartweb-service-backend-service \
    --instance-group elastos-smartweb-service-group --instance-group-zone us-east4-c --global;
gcloud compute url-maps create elastos-smartweb-service-map --default-service \
    elastos-smartweb-service-backend-service;
gcloud compute target-https-proxies create https-lb-proxy  --url-map=elastos-smartweb-service-map \
    --ssl-certificates=elastos-smartweb-service-certificate --global;
gcloud compute forwarding-rules create https-content-rule --address \
    `gcloud compute addresses describe  gce-ingress --global --format="value(address)"` \
    --global --target-https-proxy https-lb-proxy --ports 443;


#### Remove everything:
gcloud compute forwarding-rules delete https-content-rule  -q --global;
gcloud compute target-https-proxies delete https-lb-proxy -q;
gcloud compute url-maps delete elastos-smartweb-service-map -q;
gcloud compute backend-services remove-backend elastos-smartweb-service-backend-service \
    --instance-group=elastos-smartweb-service-group --global --instance-group-zone us-east4-c -q;
gcloud compute backend-services delete elastos-smartweb-service-backend-service --global -q;
#gcloud beta compute health-checks delete webpool-basic-check -q;
gcloud compute  firewall-rules delete firewall-rules-grpc -q;
gcloud compute  firewall-rules delete firewall-rules-allow-test-grpc -q;
gcloud compute instance-groups managed delete elastos-smartweb-service-group --zone us-east4-c -q;
gcloud compute instance-templates delete elastos-smartweb-service-template -q;
