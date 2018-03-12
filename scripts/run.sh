export K1TOOLS_DIR="/usr/local/k1tools"
export OUTDIR=output/bin/

$K1TOOLS_DIR/bin/k1-jtag-runner        \
	--multibinary=$OUTDIR/test.img \
	--exec-multibin=IODDR0
