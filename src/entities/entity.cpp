#include "entities/entity.h"
#include "assets/asset_paths.h"
#include "drawing/drawing.h"

Entity::Entity(
    b2WorldId world,
    b2Polygon polygon,
    b2Vec2 position,
    b2BodyDef body_def,
    b2ShapeDef shape_def,
    SDL_FColor hitbox_color,
    SDL_Texture* texture,
    std::optional<b2Vec2> texture_size
)
    : polygon_(polygon), texture_(texture), texture_size_(texture_size),
      hitbox_color(hitbox_color) {
    body_def.position = position;
    body_id_ = b2CreateBody(world, &body_def);
    b2CreatePolygonShape(body_id_, &shape_def, &polygon_);
    position_last_real_frame_ = position;
    angle_last_real_frame_ = b2Rot_GetAngle(body_def.rotation);
}

Entity::~Entity() {
    b2DestroyBody(body_id_);
}

const b2BodyId& Entity::get_body_id() const {
    return body_id_;
}

const b2Polygon& Entity::get_polygon() const {
    return polygon_;
}

b2Vec2 Entity::get_position() const {
    return b2Body_GetPosition(body_id_);
}

b2Vec2 Entity::get_interpolated_position(float alpha) const {
    b2Vec2 current_pos = b2Body_GetPosition(body_id_);
    b2Vec2 interpolated_pos;
    interpolated_pos.x = position_last_real_frame_.x * (1.f - alpha) + current_pos.x * alpha;
    interpolated_pos.y = position_last_real_frame_.y * (1.f - alpha) + current_pos.y * alpha;
    return interpolated_pos;
}

b2Rot Entity::get_interpolated_rotation(float alpha) const {
    b2Rot current_rot = b2Body_GetRotation(body_id_);
    b2Rot rotation_last_real_frame = b2MakeRot(angle_last_real_frame_);
    b2Rot interpolated_rot = b2NLerp(rotation_last_real_frame, current_rot, alpha);
    return interpolated_rot;
}

void Entity::save_previous_state() {
    position_last_real_frame_ = b2Body_GetPosition(body_id_);
    angle_last_real_frame_ = b2Rot_GetAngle(b2Body_GetRotation(body_id_));
}

bool Entity::draw(
    WindowManager& window,
    float alpha,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    AssetManager& assets
) const {
    if (!texture_ && !nametag_) {
        return false;
    }
    b2Vec2 pos = get_interpolated_position(alpha);
    b2Rot rot = get_interpolated_rotation(alpha);
    if (texture_) {
        drawing::texture(
            texture_,
            window,
            pos,
            texture_size_.value_or(b2Vec2{1.f, 1.f}),
            camera_scale,
            camera_offset_pixels,
            drawing::b2_rot_to_sdl_angle(rot)
        );
    }
    if (nametag_) {
        drawing::text(
            nametag_.get(),
            window,
            get_nametag_world_pos(alpha),
            camera_scale,
            camera_offset_pixels,
            assets.text_render_scale,
            assets.text_world_size_multiplier
        );
    }
    return true;
}

bool Entity::draw_nametag(
    WindowManager& window,
    float alpha,
    float camera_scale,
    WindowVec2 camera_offset_pixels,
    AssetManager& assets
) const {
    if (!nametag_) {
        return false;
    }
    drawing::text(
        nametag_.get(),
        window,
        get_nametag_world_pos(alpha),
        camera_scale,
        camera_offset_pixels,
        assets.text_render_scale,
        assets.text_world_size_multiplier
    );
    return true;
}

void Entity::draw_hitbox(
    WindowManager& window, float alpha, float camera_scale, WindowVec2 camera_offset_pixels
) const {
    b2Transform transform;
    transform.p = get_interpolated_position(alpha);
    transform.q = get_interpolated_rotation(alpha);
    drawing::polygon_borders(
        polygon_, window, transform, camera_scale, camera_offset_pixels, hitbox_color
    );
}

void Entity::teleport(b2Vec2 location) {
    b2Body_SetLinearVelocity(body_id_, b2Vec2{0.f, 0.f});
    b2Body_SetAngularVelocity(body_id_, 0.f);
    // b2Rot_identity is default rotation
    b2Body_SetTransform(body_id_, location, b2Rot_identity);
    b2Body_SetAwake(body_id_, true);
    position_last_real_frame_ = location;
    angle_last_real_frame_ = b2Rot_GetAngle(b2Rot_identity);
}

void Entity::set_nametag(std::string_view text, AssetManager& assets) {
    if (text.empty()) {
        if (nametag_) {
            nametag_ = nullptr;
        }
        return;
    }
    nametag_ = assets.get_sdl_text(text, asset_paths::fonts::consolas, 20.f);
}

b2Vec2
Entity::get_nametag_world_size(float text_render_scale, float text_world_size_multiplier) const {
    return drawing::get_text_world_size(
        nametag_.get(), text_render_scale, text_world_size_multiplier
    );
}

b2Vec2 Entity::get_nametag_world_pos(float alpha) const {
    b2Vec2 pos = get_interpolated_position(alpha);
    b2AABB aabb = b2Body_ComputeAABB(body_id_);
    pos.y += (aabb.upperBound.y - aabb.lowerBound.y);
    return pos;
}

std::string Entity::get_nametag_str() const {
    if (!nametag_) {
        return "";
    }
    return std::string{nametag_->text};
}