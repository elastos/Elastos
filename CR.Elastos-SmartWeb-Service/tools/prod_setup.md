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
gcloud auth login;
gcloud auth configure-docker;
gcloud compute addresses create gce-ingress --global;
gcloud compute addresses describe  gce-ingress --global --format="value(address)";
gcloud beta compute ssl-certificates create elastos-smartweb-service-certificate \
    --domains elastos-smartweb-service.nucleusconsole.com --description "Certificate for Elastos Smartweb Service";

## Repeat
# Docker
docker build -t gcr.io/careful-pillar-269322/elastos-smartweb-service -f prod.Dockerfile .;
docker tag gcr.io/careful-pillar-269322/elastos-smartweb-service gcr.io/careful-pillar-269322/elastos-smartweb-service:latest
docker push gcr.io/careful-pillar-269322/elastos-smartweb-service:latest;

# Create network and subnet
gcloud compute networks create elastos-smartweb-service-network --subnet-mode=custom
gcloud compute networks subnets create us-subnet --network=elastos-smartweb-service-network --range=10.1.10.0/24 \
    --region=us-east4 
# Configure firewall rules
gcloud compute firewall-rules create fw-allow-ssh --network=elastos-smartweb-service-network \
    --action=allow --direction=ingress --target-tags=elastos-smartweb-service-server --rules=tcp:22
gcloud compute firewall-rules create fw-allow-health-check-and-proxy --network=elastos-smartweb-service-network \
    --action=allow --direction=ingress --target-tags=elastos-smartweb-service-server \
    --source-ranges=10.1.10.0/24,130.211.0.0/22,35.191.0.0/16 --rules=tcp:8001
    
# Instance setup
gcloud compute instance-templates create-with-container elastos-smartweb-service-template \
   --machine-type g1-small --tags elastos-smartweb-service-server --network=elastos-smartweb-service-network \
   --subnet=us-subnet --container-image="gcr.io/elastos-smartweb-service/elastos-smartweb-service" \
   --container-env-file=.env
gcloud compute instance-groups managed create elastos-smartweb-service-group \
    --base-instance-name elastos-smartweb-service --zone us-east4-c --size 2 \
    --template elastos-smartweb-service-template;
       
# Define http service
gcloud compute instance-groups managed set-named-ports elastos-smartweb-service-group \
    --named-ports=grpc-port:8001 --zone us-east4-c;
# Create a health check 
gcloud compute health-checks create tcp elastos-smartweb-service-health-check --port=8001
# Create a backend service
gcloud compute backend-services create elastos-smartweb-service-backend-service \
    --port-name=grpc-port --protocol=http2 --health-checks=elastos-smartweb-service-health-check --global;  
# Add instance group as backends to backend service
gcloud compute backend-services add-backend  elastos-smartweb-service-backend-service \
    --instance-group elastos-smartweb-service-group --instance-group-zone us-east4-c --global;
# Create a URL map to route the incoming requests to appropriate backend service
gcloud compute url-maps create elastos-smartweb-service-map --default-service \
    elastos-smartweb-service-backend-service;
# Create a target HTTPS proxy to route requests to the URL map
gcloud compute target-https-proxies create elastos-smartweb-service-proxy-service \
    --url-map elastos-smartweb-service-map --ssl-certificates=elastos-smartweb-service-certificate;
# Create global forwarding rule
gcloud compute forwarding-rules create elastos-smartweb-service-forwarding-rule --address \
    `gcloud compute addresses describe  gce-ingress --global --format="value(address)"` \
    --global --target-https-proxy elastos-smartweb-service-proxy-service --ports 443;


#### Remove everything:
gcloud compute forwarding-rules delete elastos-smartweb-service-forwarding-rule -q --global;
gcloud compute target-https-proxies delete elastos-smartweb-service-proxy-service -q;
gcloud compute url-maps delete elastos-smartweb-service-map -q;
gcloud compute backend-services remove-backend elastos-smartweb-service-backend-service \
    --instance-group=elastos-smartweb-service-group --global --instance-group-zone us-east4-c -q;
gcloud compute backend-services delete elastos-smartweb-service-backend-service --global -q;
gcloud beta compute health-checks delete elastos-smartweb-service-health-check -q;
gcloud compute instance-groups managed delete elastos-smartweb-service-group --zone us-east4-c -q;
gcloud compute instance-templates delete elastos-smartweb-service-template -q;
gcloud compute firewall-rules delete fw-allow-health-check-and-proxy -q;
gcloud compute firewall-rules delete fw-allow-ssh -q;
gcloud compute networks subnets delete us-subnet --region=us-east4
gcloud compute networks delete elastos-smartweb-service-network

### Debug
PGDATABASE=smartweb_service gcloud sql connect elastos-smartweb-service --user=postgres --quiet
gcloud compute instances list 
gcloud compute instances describe instance_name
gcloud compute ssh instance_name
gcloud compute instance-groups managed recreate-instances elastos-smartweb-service-group \
    --instances=instance_name --zone us-east4-c