#pragma once

#include <stratosphere/fssrv/fssrv_interface_adapters.hpp>
#include <switch.h>

// I dunno what this is
extern "C" u64 __nx_vi_layer_id;

class IpsService : public ams::sf::IServiceObject {

private:
	enum CommandId {
		// This can only be run once, it sets things such as the layer and otherwise
		SetupOverlay = 1,
		// This allows you to set the width, height, x, and y of the overlay right now
		StartWriteOverlay = 2,
		// Write a specific number of bytes
		WriteOverlayChunk = 3,
		// Finish it off
		EndWriteOverlay = 4,
	};

	// Error handling for everything
	Result rc;

	// Overlay stuff
	ViDisplay display;
	ViLayer layer;
	NWindow window;
	Framebuffer framebuf;
	// Current pointer to the graphics data
	u8* currentBuffer;

public:
	IpsService();

	ams::Result SetupOverlay(s8 layer);

	ams::Result StartWriteOverlay(s16 width, s16 height, s16 x, s16 y);

	ams::Result WriteOverlayChunk(s32 bytesToWrite);

	ams::Result EndWriteOverlay();

	~IpsService();

	/*
		void ListMountedDrives(const ams::sf::OutArray<s32>& out_interface_ids, ams::sf::Out<s32> out_count) {
			FSP_USB_LOG("%s: forcing a mounted drive list update.", __func__);
			impl::DoUpdateDrives();

			size_t drive_count     = impl::GetAcquiredDriveCount();
			size_t buf_drive_count = std::min(drive_count, out_interface_ids.GetSize());
			FSP_USB_LOG("%s: drive count -> %lu | output drive count -> %lu.", __func__, drive_count, buf_drive_count);

			for(u32 i = 0; i < buf_drive_count; i++) {
				out_interface_ids[i] = impl::GetDriveInterfaceId(i);
				FSP_USB_LOG("%s: interface ID #%u -> %d.", __func__, i, out_interface_ids[i]);
			}

			out_count.SetValue(static_cast<s32>(buf_drive_count));
		}

		ams::Result GetDriveFileSystemType(s32 drive_interface_id, ams::sf::Out<u8> out_fs_type) {
			FSP_USB_LOG("%s (interface ID %d): forcing a mounted drive list update.", __func__, drive_interface_id);
			impl::DoUpdateDrives();
			R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveInterfaceId());

			impl::DoWithDriveFATFS(drive_interface_id, [&](FATFS* fs) {
				out_fs_type.SetValue(fs->fs_type);
				FSP_USB_LOG("%s (interface ID %d): set filesystem type to 0x%02X.", __func__, drive_interface_id, fs->fs_type);
			});

			return ams::ResultSuccess();
		}

		ams::Result GetDriveLabel(s32 drive_interface_id, ams::sf::OutBuffer& out_label_str) {
			FSP_USB_LOG("%s (interface ID %d): forcing a mounted drive list update.", __func__, drive_interface_id);
			impl::DoUpdateDrives();
			R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveInterfaceId());

			auto drive_mounted_idx = impl::GetDriveMountedIndex(drive_interface_id);
			FSP_USB_LOG("%s (interface ID %d): drive index -> %u.", __func__, drive_interface_id, drive_mounted_idx);

			char mountname[0x10] = { 0 };
			impl::FormatDriveMountName(mountname, drive_mounted_idx);
			FSP_USB_LOG("%s (interface ID %d): drive mount name -> \"%s\".", __func__, drive_interface_id, mountname);

			auto ffrc = f_getlabel(mountname, reinterpret_cast<char*>(out_label_str.GetPointer()), nullptr);
			FSP_USB_LOG("%s (interface ID %d): f_getlabel returned %u.", __func__, drive_interface_id, ffrc);

			return result::CreateFromFRESULT(ffrc);
		}

		ams::Result SetDriveLabel(s32 drive_interface_id, ams::sf::InBuffer& label_str) {
			FSP_USB_LOG("%s (interface ID %d): forcing a mounted drive list update.", __func__, drive_interface_id);
			impl::DoUpdateDrives();
			R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveInterfaceId());

			auto drive_mounted_idx = impl::GetDriveMountedIndex(drive_interface_id);
			FSP_USB_LOG("%s (interface ID %d): drive index -> %u.", __func__, drive_interface_id, drive_mounted_idx);

			char mountname[0x10] = { 0 };
			impl::FormatDriveMountName(mountname, drive_mounted_idx);
			FSP_USB_LOG("%s (interface ID %d): drive mount name -> \"%s\".", __func__, drive_interface_id, mountname);

			// Check that no more than 11 characters are copied
			// Also allow an empty label_str (label removal)
			char newname[0x100]     = { 0 };
			const char* input_label = reinterpret_cast<const char*>(label_str.GetPointer());

			if(strlen(input_label) > 0) {
				char label[0x10] = { 0 };
				snprintf(label, 11, input_label);
				sprintf(newname, "%s%s", mountname, label);
			} else {
				sprintf(newname, mountname);
			}

			FSP_USB_LOG("%s (interface ID %d): drive label -> \"%s\".", __func__, drive_interface_id, newname);

			auto ffrc = f_setlabel(newname);
			FSP_USB_LOG("%s (interface ID %d): f_setlabel returned %u.", __func__, drive_interface_id, ffrc);

			return result::CreateFromFRESULT(ffrc);
		}

		ams::Result OpenDriveFileSystem(s32 drive_interface_id, ams::sf::Out<std::shared_ptr<IFileSystemInterface>> out_fs) {
			FSP_USB_LOG("%s (interface ID %d): forcing a mounted drive list update.", __func__, drive_interface_id);
			impl::DoUpdateDrives();
			R_UNLESS(impl::IsDriveInterfaceIdValid(drive_interface_id), ResultInvalidDriveInterfaceId());

			std::shared_ptr<ams::fs::fsa::IFileSystem> drv_fs = std::make_shared<DriveFileSystem>(drive_interface_id);
			out_fs.SetValue(std::make_shared<IFileSystemInterface>(std::move(drv_fs), false));
			FSP_USB_LOG("%s (interface ID %d): IFileSystem object created.", __func__, drive_interface_id);

			return ams::ResultSuccess();
		}
		*/

	// Corresponds to each CommandId
	DEFINE_SERVICE_DISPATCH_TABLE {
		MAKE_SERVICE_COMMAND_META(SetupOverlay),
		MAKE_SERVICE_COMMAND_META(StartWriteOverlay),
		MAKE_SERVICE_COMMAND_META(WriteOverlayChunk),
		MAKE_SERVICE_COMMAND_META(EndWriteOverlay),
	};
};