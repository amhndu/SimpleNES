FROM gitpod/workspace-full-vnc
                    
USER gitpod

RUN sudo apt-get -q update && \
    sudo apt-get install -yq libsfml-dev
