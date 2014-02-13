# ChangeLog

## [2.5.0](https://github.com/commtech/fscc-linux/releases/tag/v2.5.0) (02/13/2014)
- Updated SerialFC to
[v2.3.0](https://github.com/commtech/serialfc-linux/releases/tag/v2.3.0)
- Added support for FSCC/4-PCIe cards
- Fixed issue with setting clock frequency on 4-port cards
- Minor optimizations

## [2.4.9](https://github.com/commtech/fscc-linux/releases/tag/v2.4.9) (01/10/2014)
- Added checks to prevent status/timestamp while in streaming mode
- Added clock frequency range checks
- Reimplement sysfs support for quantity input/output frames
- Minor improvements

## [2.4.8](https://github.com/commtech/fscc-linux/releases/tag/v2.4.8) (12/09/2013)
- Updated SerialFC to
[v2.2.0](https://github.com/commtech/serialfc-linux/releases/tag/v2.2.0)
- Increased the default timeout duration to 50 cycles
- Added early support for getnstimeofday()
- Fixed setting the clock rate on 232 cards
- Fixed a buf where DMA frames were getting lost
- Various small fixes

## [2.4.7](https://github.com/commtech/fscc-linux/releases/tag/v2.4.7) (11/14/2013)
- Fixed a bug that prevented small frame sizes from being calculated  correctly when approaching the memory limit

## [2.4.6](https://github.com/commtech/fscc-linux/releases/tag/v2.4.6) (11/5/2013)
- Fixed a bug where outgoing frames wouldn't get cleared fast enough

## [2.4.5](https://github.com/commtech/fscc-linux/releases/tag/v2.4.5) (10/7/2013)
- Fixed a bug where input stream data wasn't being calculated in memory usage
- Fixed tx_modifiers ioctl to accept raw integers

## [2.4.4](https://github.com/commtech/fscc-linux/releases/tag/v2.4.4) (09/25/2013)
- Fixed a regression where card's weren't detected on kernels < (3.8)

## [2.4.3](https://github.com/commtech/fscc-linux/releases/tag/v2.4.3) (09/25/2013)
- Performance improvements when transmitting and receiving frames
- Fix bug preventing tx_modifiers from being set correctly
- Remove old CLI from project
- Add links to external libraries
- Fixed a bug where card won't be recognized if serialfc driver is loaded first
- Remove hot_plug option

## [2.4.2](https://github.com/commtech/fscc-linux/releases/tag/v2.4.1) (08/28/2013)
- Fixed a bug that would allow you to return more data than your buffer  size (introduced in 2.4.0)

## [2.4.1](https://github.com/commtech/fscc-linux/releases/tag/v2.4.1) (08/28/2013)
- Stability improvements

## [2.4.0](https://github.com/commtech/fscc-linux/releases/tag/v2.4.0) (08/26/2013)
- Added support for timestamps

## [2.3.0](https://github.com/commtech/fscc-linux/releases/tag/v2.3.0) (08/14/2013)
- Now use serialfc driver for asynchronous data
- Optimize write handling
- Added sysfs support for returning multiple frames

## [2.2.4](https://github.com/commtech/fscc-linux/releases/tag/v2.2.4) (06/20/2013)
- Added support for returning multiple frames per read call
- Added support for new card ID's

## [2.2.3](https://github.com/commtech/fscc-linux/releases/tag/v2.2.3) (06/13/2013)
- Fixed a bug where having NTB set wouldn't put you in a frame based mode
- Fixed a compiler issue on older kernels
- Several small performance improvements when handling incoming frames

## [2.2.2](https://github.com/commtech/fscc-linux/releases/tag/v2.2.2) (03/25/2013)
- Performance improvements
- Fixed a bug where asynchronous baud rates were off
- Fixed compiler errors introduced in linux 3.8
- Fixed random crash in framing based modes

## [2.2.1](https://github.com/commtech/fscc-linux/releases/tag/v2.2.1) (11/16/2012)
- Fixed a bug where X-Sync mode wasn't considered stream data if there wasn't any framing
- Fixed a data loss bug in transparent mode
- Improved debug prints

## [2.2.0](https://github.com/commtech/fscc-linux/releases/tag/v2.2.0) (10/09/2012)
- Added install option to Makefile
- Improve debug prints
- Fixed typos
- Include the gui, cli and pyfscc into the project
- Added some 'get' ioctls
- Added additional examples
- Rename flush{tx,rx} to purge{tx,rx}
- Update licenses
- Added fscc.rules file
- Fixed a bug causing the number of frames to be incorrect
- Added driver specific registers to sysfs when in debug mode
- Fixed an issue where a frame was left in the FIFO
- Optimize the memory consumtion while in streaming mode
- Fixed setting the clock on older green cards.

## [2.1.3](https://github.com/commtech/fscc-linux/releases/tag/v2.1.3) (02/06/2012)
- Fixed typos
- Remove sudo from install_tools
- Remove make all
- Added default transmit_modifiers
- Added tx_modifiers and ignore_timeout to sysfs
- Rearrange initialization code
- Use sema_init exclusively
- Clean up code
- See changesets for full list of changes

## [2.1.2](https://github.com/commtech/fscc-linux/releases/tag/v2.1.2) (09/14/2011)
- init_MUTEX(x) removed in 2.6.37, I replaced it with sema_init(x,1)

## [2.1.1](https://github.com/commtech/fscc-linux/releases/tag/v2.1.1) (07/19/2011)
- Fixed bug that caused the SuperFSCC/4-LVDS to not install correctly

## [2.1.0](https://github.com/commtech/fscc-linux/releases/tag/v2.1.0) (05/02/2011)
- Fixed comments in code
- Added make help target
- Migrate from google code to bitbucket
- Added code and examples for various transmit modes
- Fixed dma insmod parameter
- Added SuperFSCC-LVDS and SuperFSCC/104 to the driver
- See changesets for full list of changes

## [2.0.5](https://github.com/commtech/fscc-linux/releases/tag/v2.0.5) (02/17/2011)
- Added a force_fifo option that prevents DMA operation
- Added a simple example showing how to use the select() function to determine how much data is available in the card

## [2.0.4](https://github.com/commtech/fscc-linux/releases/tag/v2.0.4) (02/09/2011)
- Fixed a build error with kernels prior to 2.6.27 that used an older dma mapping function than we currently use
- Added a makefile command for removing the C headers
- Added a script to soft link against our sysfs directory
- Added a script to easily download related software
- Updated the README to address some confusing sections

## [2.0.3](https://github.com/commtech/fscc-linux/releases/tag/v2.0.3) (01/13/2011)
- Don't consider being in transparent mode as necessarily being streaming data

## [2.0.2](https://github.com/commtech/fscc-linux/releases/tag/v2.0.2) (01/12/2011)
- Synchronize version number of module with repo.

## [2.0.1](https://github.com/commtech/fscc-linux/releases/tag/v2.0.1) (01/12/2011)
- Added memory constraint checks when in transparent mode
- Use atomic memory allocation when in transparent mode

## [v2.0.0](https://github.com/commtech/fscc-linux/releases/tag/v2.0.0) (01/07/2011)
This is the initial release of the 2.0 driver series.
